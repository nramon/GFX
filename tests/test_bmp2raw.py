"""Tests for util/bmp2raw.py, which is a script, so it is run as a subprocess."""
import subprocess
import sys
from pathlib import Path

import pytest
from PIL import Image

SCRIPT = Path(__file__).resolve().parent.parent / "util" / "bmp2raw.py"

# Pixel values covering the interesting range, including values above 127.
PIXELS = [0, 1, 127, 128, 200, 255, 3, 4]


def run(tmp_path, *args):
    """Run bmp2raw.py inside tmp_path and return the completed process."""
    return subprocess.run(
        [sys.executable, str(SCRIPT), *args],
        cwd=tmp_path, capture_output=True, text=True, check=False,
    )


def make_paletted_bmp(path, palette):
    """Write a 4x2 8-bit paletted BMP with the given palette."""
    img = Image.new("P", (4, 2))
    img.putpalette(palette)
    img.putdata(PIXELS)
    img.save(path)


def test_no_arguments_prints_usage(tmp_path):
    result = run(tmp_path)
    assert result.returncode == 1
    assert "Usage" in result.stderr


def test_too_many_arguments_prints_usage(tmp_path):
    result = run(tmp_path, "a.bmp", "b.bmp")
    assert result.returncode == 1
    assert "Usage" in result.stderr


def test_missing_file_fails(tmp_path):
    result = run(tmp_path, "missing.bmp")
    assert result.returncode != 0


def test_rgb_image_is_rejected(tmp_path):
    Image.new("RGB", (4, 2), (1, 2, 3)).save(tmp_path / "rgb.bmp")
    result = run(tmp_path, "rgb.bmp")
    assert result.returncode == 1
    assert "8-bit" in result.stderr
    assert not (tmp_path / "rgb.raw").exists()


def test_pixels_are_written_unchanged(tmp_path):
    make_paletted_bmp(tmp_path / "img.bmp", list(range(256)) * 3)
    assert run(tmp_path, "img.bmp").returncode == 0
    assert list((tmp_path / "img.raw").read_bytes()) == PIXELS


def test_palette_is_converted_to_6_bit(tmp_path):
    # Entry i has R=i, G=255-i, B=(i*2)%256.
    palette = []
    for i in range(256):
        palette += [i, 255 - i, (i * 2) % 256]
    make_paletted_bmp(tmp_path / "img.bmp", palette)
    assert run(tmp_path, "img.bmp").returncode == 0

    data = (tmp_path / "img.pal").read_bytes()
    assert len(data) == 768
    assert all(value <= 63 for value in data)
    # 8-bit to 6-bit VGA conversion, truncating.
    assert list(data) == [value * 63 // 255 for value in palette]


def test_output_named_after_input_in_current_directory(tmp_path):
    sub = tmp_path / "sub"
    sub.mkdir()
    make_paletted_bmp(sub / "pic.bmp", list(range(256)) * 3)
    assert run(tmp_path, "sub/pic.bmp").returncode == 0
    assert (tmp_path / "pic.raw").exists()
    assert (tmp_path / "pic.pal").exists()


@pytest.mark.xfail(strict=True, reason=(
    "bmp2raw.py checks 'pal is None' after wrapping getpalette() in "
    "numpy.array(), so an image without a palette crashes with a TypeError "
    "instead of reporting 'No palette information found'."))
def test_image_without_palette_reports_error(tmp_path):
    Image.new("L", (4, 2), 7).save(tmp_path / "gray.bmp")
    result = run(tmp_path, "gray.bmp")
    assert result.returncode == 1
    assert "No palette" in result.stderr
    assert "Traceback" not in result.stderr

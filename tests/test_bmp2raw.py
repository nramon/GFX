"""Tests for util/bmp2raw.py, which is a script, so it is run as a subprocess."""
import subprocess
import sys
from pathlib import Path

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
    # 8-bit to 6-bit VGA conversion, rounding to the nearest value.
    assert list(data) == [(value * 63 + 127) // 255 for value in palette]


def test_output_named_after_input_in_current_directory(tmp_path):
    sub = tmp_path / "sub"
    sub.mkdir()
    make_paletted_bmp(sub / "pic.bmp", list(range(256)) * 3)
    assert run(tmp_path, "sub/pic.bmp").returncode == 0
    assert (tmp_path / "pic.raw").exists()
    assert (tmp_path / "pic.pal").exists()


def test_image_without_palette_reports_error(tmp_path):
    Image.new("L", (4, 2), 7).save(tmp_path / "gray.bmp")
    result = run(tmp_path, "gray.bmp")
    assert result.returncode == 1
    assert "No palette" in result.stderr
    assert "Traceback" not in result.stderr


def test_short_palette_is_padded_to_256_colors(tmp_path):
    # A 16-color palette, with pixel values that only use those colors.
    img = Image.new("P", (4, 2))
    img.putpalette([255, 255, 255] * 16)
    img.putdata([0, 1, 2, 3, 4, 5, 6, 7])
    img.save(tmp_path / "small.bmp")
    assert run(tmp_path, "small.bmp").returncode == 0

    data = (tmp_path / "small.pal").read_bytes()
    assert len(data) == 768
    assert list(data[:48]) == [63] * 48
    assert list(data[48:]) == [0] * (768 - 48)


def test_warns_about_image_size_other_than_320x200(tmp_path):
    make_paletted_bmp(tmp_path / "img.bmp", list(range(256)) * 3)
    result = run(tmp_path, "img.bmp")
    assert result.returncode == 0
    assert "not 320x200" in result.stderr


def test_no_warning_for_320x200_image(tmp_path):
    img = Image.new("P", (320, 200))
    img.putpalette(list(range(256)) * 3)
    img.save(tmp_path / "full.bmp")
    result = run(tmp_path, "full.bmp")
    assert result.returncode == 0
    assert "not 320x200" not in result.stderr
    assert (tmp_path / "full.raw").stat().st_size == 64000

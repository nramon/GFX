# GFX

Graphics programming stuff.

VGA mode 13h demos for DOS, written in C with a bit of inline assembly and
direct access to video memory.

* `vga/`: small VGA library (palette, drawing buffer, fades...).
* `fade/`: fade effect between two images.
* `stars/`: scrolling star field.
* `util/`: helper scripts (`bmp2raw.py` needs Python with Pillow and numpy,
  `ttf2bmp.sh` needs ImageMagick).

## Building on Linux

The demos are cross-compiled to 16-bit DOS with [Open Watcom](https://github.com/open-watcom/open-watcom-v2)
and run in DOSBox. You need `make`, `curl`, `7z` (p7zip-full) and `dosbox`.

    make toolchain    # Download Open Watcom into .toolchain/ (once, ~130 MB).
    make              # Build all demos into build/.
    make run-fade     # Build and run a demo in DOSBox.
    make run-stars

The sources are built unmodified: `compat/watcom/alloc.h` maps the few
Borland-specific names they use to their Open Watcom equivalents.

## Building under DOS

With Turbo C++ 3.0 and Turbo Assembler, run `MAKE` inside `fade` or `stars`.

# GFX

Graphics programming stuff.

VGA mode 13h demos for DOS, written in C with a bit of inline assembly and
direct access to video memory.

* `vga/`: small VGA library (palette, drawing buffer, fades...).
* `fade/`: fade effect between two images.
* `stars/`: scrolling star field.
* `docs/primer.md`: a short primer on the C and assembly used here. Each demo has a
  README that works as a tutorial.
* `util/`: helper scripts (`bmp2raw.py` needs Python with Pillow and numpy,
  `ttf2bmp.sh` needs ImageMagick).

## Building on Linux

The demos are cross-compiled to 16-bit DOS with [Open Watcom](https://github.com/open-watcom/open-watcom-v2)
and run in DOSBox. You need `make`, `curl`, `7z` (p7zip-full) and `dosbox`.

    make toolchain    # Download Open Watcom (~130 MB) and shellcheck into .toolchain/.
    make              # Build all demos into build/.
    make fade         # Build and run a demo in DOSBox (also: make run-fade).
    make stars
    make build/stars.exe   # Build one demo without running it.

The sources are built unmodified: `compat/watcom/alloc.h` maps the few
Borland-specific names they use to their Open Watcom equivalents.

## Tests, lint and static analysis

    make test         # pytest suite for util/bmp2raw.py (needs pytest, Pillow, numpy).
    make lint         # flake8 (Python), shellcheck (shell) and cppcheck (C).
    make analyze      # Style findings and Open Watcom's maximum warning level.

`make lint` fails only on real problems. `make analyze` is informational: it
reports style issues and the compiler's maximum warnings. `make lint` uses the
shellcheck installed by `make toolchain`, or one in the PATH; without either it
only checks the shell syntax.

## Building under DOS

With Turbo C++ 3.0 and Turbo Assembler, run `MAKE` inside `fade` or `stars`.

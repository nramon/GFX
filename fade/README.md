# fade: palettes

A fade between two images. Run it with `make fade` from the repository root. Read
[the primer](../docs/primer.md) first if segments, far pointers or ports are new to you.

The demo loads `img1.raw`, `img1.pal` and the other image files from the current directory.
`make` takes care of that. If you run the .exe from elsewhere you get "Could not load the images."

## The idea: pixels are indices

In mode 13h each pixel is **one byte**, and that byte is not a colour. It is an
index into a table of 256 colours, the **palette**. Each palette entry is three
numbers (red, green, blue), and the VGA's DAC (digital-to-analog converter) turns
them into the signal sent to the monitor.

This has a nice consequence: change the palette and every pixel on the screen changes
at once, without touching video memory. A fade is exactly that.

## 6 bits per component

The DAC only uses the low **6 bits** of each component, so values run 0..63 and not
0..255 (`VGA_PALMAX` is 63). The palette files (`img1.pal`, `img2.pal`) hold 256 x 3 = 768
bytes, already scaled to 0..63. `util/bmp2raw.py` does the 8-bit to 6-bit conversion when it
makes them from a paletted image.

## How the demo works

Look at `fade.c`. After loading the two images and their palettes into memory it does
(simplified):

```c
vga_clear_pal();                 /* every colour is black */
vga_load_screen(img1);           /* the image is in video memory, but invisible */
vga_fade_in(pal1, FADE_DELAY);   /* reveal it */
vga_sleep(FADE_SLEEP);
vga_fade_out(pal1, FADE_DELAY);
```

The picture is copied to the screen while the palette is all black, so nobody sees it
being drawn. `vga_fade_in` (in `vga/vga.c`) then loops with a level `i` from 1 to 63. At each
step it writes every colour of the real palette, but clamps each component to at most `i`:

```c
if (pal[c][VGA_R] > i) { outportb(0x03C9, i); } else { outportb(0x03C9, pal[c][VGA_R]); }
```

Each component grows until it reaches its target, so low components finish early and high ones
last. Note that this is not a proportional fade: all components rise at the same speed, so
colours change hue on the way. For example (63, 10, 0) goes through (10, 10, 0), a yellowish
grey, before the red catches up. `vga_fade_out` does the reverse, from 62 down to 0.

## Timing

`vga_sleep(n)` waits `n` BIOS clock ticks. The BIOS timer ticks about 18.2 times a second,
so `FADE_DELAY` (1 tick) is about 55 ms per step and a full 63-step fade takes about 3.5
seconds. The tick counter is reset at midnight, so `vga_sleep` handles that case: a negative difference means
the counter wrapped and `VGA_TICKSPERDAY` is added to it.

## Making your own images

Create a 320x200 image with a palette (for example, in GIMP, use *Image > Mode > Indexed*)
and save it as BMP, then:

```
python3 util/bmp2raw.py myimage.bmp     # writes myimage.raw and myimage.pal in the current directory
```

The script warns if the size is not 320x200 and refuses images without a palette.

## Try this

* Change `FADE_DELAY` or `FADE_SLEEP` in `fade.h` and see how the timing changes.
* Make a **colour fade**: show one image and interpolate its palette, entry by entry, towards
  a different one (for example the same colours inverted) instead of going through black.
* Make the fade **proportional**: set each component to `pal[c][k] * i / 63` (the product is
  at most 63 * 63, so it fits in an `int`) and compare the hues.
* Fade to white instead of black: start with a palette of all 63s and make each component
  move towards its target.
* Write a **palette rotation**: shift the colours 16-31 by one entry every frame to animate
  water or fire without redrawing anything.

# stars: animation

A scrolling star field. Run it with `make stars` from the repository root and press a key to quit.
Read [the primer](../docs/primer.md) first if segments or ports are new to you.

## The main loop

Everything in `stars.c` is in this shape:

```c
while (!kbhit()) {
	vga_clear_buffer();          /* 1. erase the off-screen buffer */
	/* ... move and draw every star into the buffer ... */
	vga_update_screen();         /* 2. copy the buffer to the screen */
}
```

## Double buffering

Drawing straight to video memory makes flicker: the monitor can show a screen that is half
erased and half drawn. The fix is to draw each frame in an **off-screen buffer** (`VGA_BUFFER`, a
normal far-heap block of 64000 bytes) and copy it to the screen in one go. `vga_put_pixel`
writes to the buffer, and `vga_update_screen` copies it, with an `REP MOVSW` of 32000 words.

## Vertical retrace

Even the copy can tear if it happens while the monitor is drawing. A CRT redraws top to
bottom about 70 times a second and then pauses briefly while the beam returns to the top.
That pause is the **vertical retrace**, and bit 3 of port `3DA` is set during it.
`vga_update_screen` waits for it before copying:

```c
while (inportb(0x03DA) & 0x08);     /* wait for any retrace to end... */
while (!(inportb(0x03DA) & 0x08));  /* ...and the next one to start */
```

This also caps the frame rate to the monitor's refresh rate, so the animation runs at a
steady speed on any machine fast enough to draw a frame per refresh. The star speed is per
frame, so a slower machine would simply run the animation slower.

The wait mostly prevents tearing, not entirely: the retrace is very short and the copy of
64000 bytes can take longer than it on slow hardware, so the beam can overtake the copy and
the lower part of the screen shows a mix of the old and the new frame.

## Parallax

Each star has a depth from 1 (nearest) to `STAR_DEPTH`. Each frame its `x` moves by
`STAR_SPEED / depth`, so far stars crawl and near ones go by fast, which gives an illusion
of depth. When a star leaves the right edge it is reborn on the left at a random height
and depth.

The colours use the palette as well: colour indices 1 to `STAR_DEPTH` are greys set with
`vga_set_color`, and the nearest stars are the brightest. A star draws with its depth
as the colour index, so a star's depth and its colour are the same number.

## Pixels and coordinates

`vga_put_pixel(x, y, c)` ignores points outside the screen, so a star can never write
past the buffer. Inside, it computes the offset as `x + y * 320` with shifts, using
unsigned arithmetic, as explained in the primer.

## Try this

* Add a fourth depth: raise `STAR_DEPTH` to 4 and run it. The star colours are
  `VGA_PALMAX - (i << 4)`, so what happens at depth 4? (Hint: the value doesn't fit in a
  `byte_t`.) Fix the formula.
* Change the stars so they fly **towards you** from the centre of the screen. Each star would
  need an `x, y` direction away from the centre, and it would speed up as it goes.
* Make the stars leave short **trails** by dimming the buffer instead of clearing it. You
  need a palette with a gradient of greys, so that subtracting 1 from a pixel's index makes it
  darker.
* Let some keys change the speed: call `getch()` inside the loop when `kbhit()` is true, and
  quit only on Esc. Remove or adjust the `getch()` after the loop, or it will wait for a
  second keypress.

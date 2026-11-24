# vga: the library

A small library for VGA mode 13h (320x200, 256 colours): see [the primer](../docs/primer.md)
for the background. Include `vga.h`, and link `vga.obj`.

All drawing happens in an off-screen buffer (`VGA_BUFFER`) unless the function says "screen".
The buffer and the screen are both 64000 bytes: one byte per pixel, row after row.

| Function | Description |
|----------|-------------|
| `vga_init()` | Allocate the drawing buffer. Call it first. Exits with code 1 if it can't. |
| `vga_free()` | Free the buffer. |
| `vga_video_mode()` | Enter mode 13h. |
| `vga_text_mode()` | Go back to 80x25 text mode. |
| `vga_clear_buffer()` / `vga_clear_screen()` | Fill the buffer / screen with 0. |
| `vga_update_screen()` | Wait for the vertical retrace, then copy the buffer to the screen. |
| `vga_put_pixel(x, y, c)` | Draw a pixel in the buffer. Out-of-range points are ignored. |
| `vga_load_buffer(src)` / `vga_load_screen(src)` | Copy 64000 bytes from a far pointer. |
| `vga_set_color(c, r, g, b)` / `vga_get_color(...)` | Set or read one palette entry (components 0..63). |
| `vga_set_pal(pal)` / `vga_get_pal(pal)` | Set or read the whole palette (`pal_t`, 256 x 3 bytes). |
| `vga_clear_pal()` | Set every colour to black. |
| `vga_fade_in(pal, delay)` / `vga_fade_out(pal, delay)` | Fade to or from a palette, `delay` ticks per step. |
| `vga_sleep(ticks)` | Wait for BIOS clock ticks (about 18.2 per second). |
| `vga_read_buffer(fname, buf)` / `vga_write_buffer(...)` | Load or save 64000 raw bytes. Return 0, or -1 on error. |
| `vga_read_pal(fname, pal)` / `vga_write_pal(...)` | Load or save a 768-byte palette. Return 0, or -1 on error. |

## Minimal program

```c
#include <conio.h>
#include "vga.h"

void main() {
	int x;

	vga_init();
	vga_clear_buffer();                  /* the buffer is uninitialised */
	vga_video_mode();
	vga_set_color(1, 63, 0, 0);          /* colour 1 = red */
	for (x = 0; x < VGA_RESX; x++) {
		vga_put_pixel(x, 100, 1);    /* a horizontal line */
	}
	vga_update_screen();
	getch();
	vga_text_mode();
	vga_free();
}
```

A new demo needs a directory with a `.c` file, an entry in `DEMOS` and `C_SOURCES` in the root
`Makefile`, and rules for its object file and executable (copy the `stars` ones). To build under DOS,
copy a demo's `Makefile`.

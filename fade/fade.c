#include <alloc.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vga.h"
#include "fade.h"

/*
 * Fade effect between two images.
 */
void main() {
	int k;
	long int i, j;
	byte_t far *img1, far *img2;
	pal_t pal1, pal2;

	/* VGA initialization. */
	vga_init();

	/* Allocate memory for the image buffers. */
	img1 = (byte_t far *)farmalloc(VGA_BUFFSIZE);
	if (img1 == NULL) {
		exit(1);
	}
	img2 = (byte_t far *)farmalloc(VGA_BUFFSIZE);
	if (img2 == NULL) {
		farfree((void *)img1);
		exit(1);
	}

	/* Load the two images and their respective palettes.*/
	vga_read_buffer("img1.raw", img1);
	vga_read_pal("img1.pal", pal1);
	vga_read_buffer("img2.raw", img2);
	vga_read_pal("img2.pal", pal2);

	/* Enter video mode and clear the screen, buffer and palette. */
	vga_video_mode();
	vga_clear_screen();
	vga_clear_buffer();
	vga_clear_pal();

	/* Load the first image, fade in, sleep and fade out. */
	vga_load_screen(img1);
	vga_fade_in(pal1, FADE_DELAY);
	vga_sleep(FADE_SLEEP);
	vga_fade_out(pal1, FADE_DELAY);

	/* Load the second image, fade in, sleep and fade out. */
	vga_load_screen(img2);
	vga_fade_in(pal2, FADE_DELAY);
	vga_sleep(FADE_SLEEP);
	vga_fade_out(pal2, FADE_DELAY);

	/* Clean-up. */
	farfree((void far *)img2);
	farfree((void far *)img1);
	vga_clear_screen();
	vga_text_mode();
	vga_free();
}

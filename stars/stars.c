#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include "vga.h"
#include "stars.h"

/******************************************************************************
 * Scrolling star field effect.                                               *
 ******************************************************************************/
void main() {
	int i;
	star_t stars[STAR_NUM];

	/* VGA initialization. */
	vga_init();

	/* Initialize stars at random. */
	srand((unsigned int)time(NULL));
	for (i = 0; i < STAR_NUM; i++) {
		stars[i].x = rand() % VGA_RESX;
		stars[i].y = rand() % VGA_RESY;
		stars[i].depth = 1 + rand() % STAR_DEPTH;
	}

	/* Enter video mode and clear the screen. */
	vga_video_mode();
	vga_clear_screen();

	/* Set the background to black. */
	vga_set_color(0, 0, 0, 0);

	/* Set per-depth star colors. */
	for (i = 1; i <= STAR_DEPTH; i++) {
		byte_t c = VGA_PALMAX - (i << 4); /* Shift left to separate the colors. */
		vga_set_color(i, c, c, c);
	}

	/* Loop until a key is pressed. */
	while (!kbhit()) {

		/* Clear the drawing buffer. */
		vga_clear_buffer();

		/* Update star positions. */
		for (i = 0; i < STAR_NUM; i++) {
			stars[i].x += STAR_SPEED / stars[i].depth;

			/* The star left the screen. Create a new random star. */
			if (stars[i].x >= VGA_RESX) {
				stars[i].x = 0;
				stars[i].y = rand() % VGA_RESY;
				stars[i].depth = 1 + rand() % STAR_DEPTH;
			}

			/* Draw the star. */
			vga_put_pixel((int)(stars[i].x), (int)(stars[i].y), stars[i].depth);
		}

		/* Copy the drawing buffer to the screen. */
		vga_update_screen();
	}

	/* Clean-up. Consume the key that was pressed. */
	getch();
	vga_clear_screen();
	vga_text_mode();
	vga_free();
}

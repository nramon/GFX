#include <alloc.h>
#include <bios.h>
#include <dos.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vga.h"

/* Video memory and drawing buffer. */
byte_t far *VGA_SCREEN = (byte_t far *)MK_FP(0xA000, 0x0000);
byte_t far *VGA_BUFFER = NULL;

/*
 * Clear the drawing buffer.
 */
void vga_clear_buffer() {
	/* _fmemset(VGA_BUFFER, 0, VGA_BUFFSIZE); */
	asm {
		PUSH DS
		XOR AX, AX
		LES DI, DWORD PTR [VGA_BUFFER]
		MOV CX, 32000d
		REP STOSW
		POP DS
	};
}

/*
 * Clear the palette.
 */
void vga_clear_pal() {
	int c;

	for (c = 0; c < VGA_COLNUM; c++) {
		outportb(0x03C8, c); /* Select the color. */
		outportb(0x03C9, 0); /* Zero the R value. */
		outportb(0x03C9, 0); /* Zero the G value. */
		outportb(0x03C9, 0); /* Zero the B value. */
	}
}

/*
 * Clear the string.
 */
void vga_clear_screen() {
	/* _fmemset(VGA_SCREEN, 0, VGA_BUFFSIZE); */
	asm {
		PUSH DS
		XOR AX, AX
		LES DI, DWORD PTR [VGA_SCREEN]
		MOV CX, 32000d
		REP STOSW
		POP DS
	};
}

/*
 * Palette fade in.
 */
void vga_fade_in(pal_t pal, clock_t delay) {
	int i, c;

	/* Increase color components until they match the given palette.
	   Start at 1, since 0 values will not be increased. */
	for (i = 1; i <= VGA_PALMAX; i++) {
		for (c = 0; c < VGA_COLNUM; c++) {

			/* Select the color. */
			outportb(0x03C8, c);

			/* Increase R values lower than the target value. */
			if (pal[c][VGA_R] > i) {
				outportb(0x03C9, i);
			} else {
				outportb(0x03C9, pal[c][VGA_R]);
			}

			/* Increase G values lower than the target value. */
			if (pal[c][VGA_G] > i) {
				outportb(0x03C9, i);
			} else {
				outportb(0x03C9, pal[c][VGA_G]);
			}

			/* Increase B values lower than the target value. */
			if (pal[c][VGA_B] > i) {
				outportb(0x03C9, i);
			} else {
				outportb(0x03C9, pal[c][VGA_B]);
			}
		}

		/* Go to sleep. */
		vga_sleep(delay);
	}
}

/*
 * Palette fade out.
 */
void vga_fade_out(pal_t pal, clock_t delay) {
	int i, c;

	/* Decrease color components until they reach 0.
	   Start at VGA_PALMAX - 1, since VGA_PALMAX values will not be decreased. */
	for (i = VGA_PALMAX - 1; i >= 0; i--) {
		for (c = 0; c < VGA_COLNUM; c++) {

			/* Select the color. */
			outportb(0x03C8, c);

			/* Decrease R values higher than the desired value. */
			if (pal[c][VGA_R] > i) {
			    outportb(0x03C9, i);
			} else {
				outportb(0x03C9, pal[c][VGA_R]);
			}

			/* Decrease G values higher than the desired value. */
			if (pal[c][VGA_G] > i) {
			    outportb(0x03C9, i);
			} else {
				outportb(0x03C9, pal[c][VGA_G]);
			}

			/* Decrease B values higher than the desired value. */
			if (pal[c][VGA_B] > i) {
			    outportb(0x03C9, i);
			} else {
				outportb(0x03C9, pal[c][VGA_B]);
			}
		}

		/* Go to sleep. */
		vga_sleep(delay);
	}
}

/*
 * Clean up stuff.
 */
void vga_free() {
	farfree((void far *)VGA_BUFFER);
	VGA_BUFFER = NULL;
}

/*
 * Get the (r,g,b) values for color c.
 */
void vga_get_color(byte_t c, byte_t *r, byte_t *g, byte_t *b) {
	outportb(0x03C7, c);  /* Select the color. */
	*r = inportb(0x03C9); /* Get the R value. */
	*g = inportb(0x03C9); /* Get the G value. */
	*b = inportb(0x03C9); /* Get the B value. */
}

/*
 * Read the whole palette.
 */
void vga_get_pal(pal_t pal) {
	int c;

	for (c = 0; c < VGA_COLNUM; c++) {
		outportb(0x03C7, c);         /* Select the color. */
		pal[c][VGA_R] = inportb(0x03C9); /* Get the R value. */
		pal[c][VGA_G] = inportb(0x03C9); /* Get the G value. */
		pal[c][VGA_B] = inportb(0x03C9); /* Get the B value. */
	}
}

/*
 * Initialization stuff.
 */
void vga_init() {
	VGA_BUFFER = (byte_t far *)farmalloc(VGA_BUFFSIZE);
	if (VGA_BUFFER == NULL) {
		exit(1);
	}
}

/*
 * Load data into the drawing buffer.
 */
void vga_load_buffer(byte_t far *src) {
	/* _fmemcpy(VGA_BUFFER, src, VGA_BUFFSIZE); */
	asm {
		PUSH DS
		LES DI, DWORD PTR [VGA_BUFFER]
		LDS SI, DWORD PTR [src]
		MOV CX, 32000d
		REP MOVSW
		POP DS
	};
}

/*
 * Load data directly into the screen.
 */
void vga_load_screen(byte_t far *src) {
	/* _fmemcpy(VGA_SCREEN, src, VGA_BUFFSIZE); */
	asm {
		PUSH DS
		LES DI, DWORD PTR [VGA_SCREEN]
		LDS SI, DWORD PTR [src]
		MOV CX, 32000d
		REP MOVSW
		POP DS
	};
}

/*
 * Draw a pixel to the buffer at (x, y) with color c.
 */
void vga_put_pixel(int x, int y, byte_t c) {
	/* VGA_BUFFER[y * VGA_RESX + x] = c; */
	VGA_BUFFER[x + (y << 8) + (y << 6)] = c;
}

/*
 * Read buffer data from disk.
 */
int vga_read_buffer(char *fname, byte_t far *buffer) {
	long int i;
	byte_t tmp_buffer[VGA_TMPBUFFSIZE];
	FILE *fh;

	/* Open the target file. */
	fh = fopen(fname, "rb");
	if (fh == NULL) {
		return -1;
	 }

	/* Write buffer data to disk in chunks. */
	for (i = 0; i < VGA_BUFFSIZE / VGA_TMPBUFFSIZE; i++) {
		if (fread(tmp_buffer, VGA_TMPBUFFSIZE, 1, fh) != 1) {
			fclose(fh);
			return -1;
		}
		_fmemcpy(buffer + i * VGA_TMPBUFFSIZE, tmp_buffer, VGA_TMPBUFFSIZE);
	}

	/* Clean up. */
	fclose(fh);

	return 0;
}

/*
 * Read palette data from disk.
 */
int vga_read_pal(char *fname, pal_t pal) {
	int i;
	byte_t r, g, b;
	FILE *fh;

	/* Open the target file. */
	fh = fopen(fname, "rb");
	if (fh == NULL) {
		return -1;
	 }

	/* Read the whole palette. */
	if (fread((void *)pal, VGA_COLNUM * VGA_RGBNUM, 1, fh) != 1) {
		fclose(fh);
		return -1;
	}

	/* Clean up. */
	fclose(fh);

	return 0;
}

/*
 * Set the (r,g,b) values for color c.
 */
void vga_set_color(byte_t c, byte_t r, byte_t g, byte_t b) {
	outportb(0x03C8, c); /* Select the color. */
	outportb(0x03C9, r); /* Set the R value. */
	outportb(0x03C9, g); /* Set the G value. */
	outportb(0x03C9, b); /* Set the B value. */
}

/*
 * Set the whole palette.
 */
void vga_set_pal(pal_t pal) {
	int c;

	for (c = 0; c < VGA_COLNUM; c++) {
		outportb(0x03C8, c);         /* Select the color. */
		outportb(0x03C9, pal[c][VGA_R]); /* Set the R value. */
		outportb(0x03C9, pal[c][VGA_G]); /* Set the G value. */
		outportb(0x03C9, pal[c][VGA_B]); /* Set the B value. */
	}
}

/*
 * Wait until the given number of clock ticks have elapsed.
 */
void vga_sleep(clock_t ticks) {
	clock_t start, stop;

	start = biostime(0, 0);
	do {
		stop = biostime(0, 0);
	} while (stop - start < ticks);
}

/*
 * Enter text mode.
 */
void vga_text_mode() {
	asm {
		MOV AX, 0003H
		INT 10H
	};
}

/*
 * Copy the drawing buffer to the screen.
 */
void vga_update_screen() {

	/* Wait for a vertical retrace. */
	while (inportb(0x03DA) & 0x08);
	while (!(inportb(0x03DA) & 0x08));

	/* _fmemcpy(VGA_SCREEN, VGA_BUFFER, VGA_BUFFSIZE); */
	asm {
		PUSH DS
		LES DI, DWORD PTR [VGA_SCREEN]
		LDS SI, DWORD PTR [VGA_BUFFER]
		MOV CX, 32000d
		REP MOVSW
		POP DS
	};
}

/*
 * Enter video mode.
 */
void vga_video_mode() {
	asm {
		MOV AX, 0013H
		INT 10H
	};
}


/*
 * Write the given buffer to disk.
 */
int vga_write_buffer(char *fname, byte_t far *buffer) {
	long int i;
	byte_t tmp_buffer[VGA_TMPBUFFSIZE];
	FILE *fh;

	/* Open the target file. */
	fh = fopen(fname, "wb");
	if (fh == NULL) {
		return -1;
	}

	/* Write buffer data to disk byte by byte (slow). */
	for (i = 0; i < VGA_BUFFSIZE / VGA_TMPBUFFSIZE; i++) {
		_fmemcpy(tmp_buffer, buffer + i * VGA_TMPBUFFSIZE, VGA_TMPBUFFSIZE);
		if (fwrite(tmp_buffer, VGA_TMPBUFFSIZE, 1, fh) != 1) {
			fclose(fh);
			return -1;
		}
	}

	/* Clean up. */
	fclose(fh);

	return 0;
}

/*
 * Write the given palette to disk.
 */
int vga_write_pal(char *fname, pal_t pal) {
	FILE *fh;

	/* Open the target file. */
	fh = fopen(fname, "wb");
	if (fh == NULL) {
		return -1;
	}

	/* Write the whole palette. */
	if (fwrite((void *)pal, VGA_COLNUM * VGA_RGBNUM, 1, fh) != 1) {
		fclose(fh);
		return -1;
	}

	/* Clean up. */
	fclose(fh);

	return 0;
}

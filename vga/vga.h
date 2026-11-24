#ifndef _VGA_H_
#define _VGA_H_

#include <time.h>

#define VGA_BUFFSIZE    64000 /* Screen buffer size. Must be equal to RES_X * RES_Y. */
#define VGA_COLMAX        255 /* Maximum value for a color. */
#define VGA_COLNUM        256 /* Number of colors available. */
#define VGA_GPMHSIZE       15 /* Size of a GPM header. */
#define VGA_PALMAX         63 /* Maximum possible value for each color component (R, G or B). */
#define VGA_RGBMAX        255 /* Maximum value for R, G and B values. */
#define VGA_RGBNUM          3 /* Number of color components. */
#define VGA_RESX          320 /* Horizontal screen resolution. */
#define VGA_RESY          200 /* Vertical screen resolution. */
#define VGA_TMPBUFFSIZE   640 /* Temporary buffer size. Must be a multiple of VGA_BUFFSIZE. */

#define VGA_R               0 /* Index of the R value in a palette. */
#define VGA_G               1 /* Index of the R value in a palette. */
#define VGA_B               2 /* Index of the R value in a palette. */

typedef unsigned char byte_t; /* A byte. */
typedef byte_t pal_t[256][3]; /* An array to hold palette information. */

/* Video memory and drawing buffer. */
extern byte_t far *VGA_SCREEN;
extern byte_t far *VGA_BUFFER;

/* VGA functions. */
void vga_clear_buffer();
void vga_clear_pal();
void vga_clear_screen();
void vga_fade_in(pal_t pal, clock_t delay);
void vga_fade_out(pal_t pal, clock_t delay);
void vga_free();
void vga_get_color(byte_t c, byte_t *r, byte_t *g, byte_t *b);
void vga_get_pal(pal_t pal);
void vga_init();
void vga_load_buffer(byte_t far *src);
void vga_load_screen(byte_t far *src);
void vga_put_pixel(int x, int y, byte_t c);
int  vga_read_buffer(char *fname, byte_t far *buffer);
int  vga_read_pal(char *fname, pal_t pal);
void vga_set_color(byte_t c, byte_t r, byte_t g, byte_t b);
void vga_set_pal(pal_t pal);
void vga_sleep(clock_t ticks);
void vga_text_mode();
void vga_update_screen();
void vga_video_mode();
int  vga_write_buffer(char *fname, byte_t far *buffer);
int  vga_write_pal(char *fname, pal_t pal);

#endif

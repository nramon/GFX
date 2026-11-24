#ifndef _STARS_H_
#define _STARS_H_

#define STAR_DEPTH    3 /* Maximum depth of a star (1 = foreground). */
#define STAR_NUM    256 /* Maximum number of stars. */
#define STAR_SPEED  1.0 /* Speed of a foreground star. */

typedef struct {
	float x, y;          /* Star (x, y) coordinates. */
	unsigned char depth; /* Star depth. */
} star_t;

#endif

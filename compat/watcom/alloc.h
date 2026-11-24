/*
 * Borland-to-Open Watcom compatibility shim.
 *
 * The sources were written for Turbo C++ and include <alloc.h> first. When
 * building with Open Watcom, this header is found instead and maps the few
 * Borland-specific names the code uses to their Watcom equivalents.
 */
#ifndef _COMPAT_WATCOM_ALLOC_H_
#define _COMPAT_WATCOM_ALLOC_H_

#include <bios.h>
#include <conio.h>
#include <dos.h>
#include <i86.h>
#include <malloc.h>

/* Inline assembler: asm { ... } -> _asm { ... }. */
#define asm _asm

/* Port I/O. */
#define outportb(port, value) outp(port, value)
#define inportb(port)         inp(port)

/* Far heap. */
#define farmalloc(size)       _fmalloc(size)
#define farfree(ptr)          _ffree(ptr)

/* BIOS timer ticks (only reading the tick count is supported). */
static __inline long biostime(int cmd, long newtime) {
	unsigned long ticks;
	(void)cmd;
	(void)newtime;
	_bios_timeofday(_TIME_GETCLOCK, (long *)&ticks);
	return (long)ticks;
}

#endif

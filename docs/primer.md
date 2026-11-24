# Primer: the C and assembly used in this repo

This is not a course on C or assembly. It covers only what the code in `vga/`,
`fade/` and `stars/` uses, so you can read it. It was written for 16-bit DOS
compilers (Turbo C++ 3.0, Open Watcom `-ms`).

## 1. Memory: segments and far pointers

An 8086 has 16-bit registers but a 20-bit address bus. An address is a pair,
`segment:offset`, and the real address is `segment * 16 + offset`. Inside one
segment you can reach at most 64 KB (the largest 16-bit offset is 65535).

* A **near** pointer is just the 16-bit offset, within the program's own data segment.
* A **far** pointer is both halves, 32 bits, and can point anywhere in the first 1 MB.

The VGA frame buffer in mode 13h lives at `A000:0000`, so the library keeps a far pointer to it:

```c
byte_t far *VGA_SCREEN = (byte_t far *)MK_FP(0xA000, 0x0000);
```

`MK_FP(seg, off)` builds a far pointer from the two halves. After that, `VGA_SCREEN[i] = c;`
writes pixel `i` straight to the screen, with no function call and no operating system in the way.
Use an `unsigned int` index for `i` above 32767, or it goes negative (see the next section).

A mode 13h screen is 320 x 200 = 64000 bytes, which is just under 64 KB (65536). That is why
a whole screen fits in one segment, and why the buffers are allocated with `farmalloc` (from
the far heap) instead of `malloc`: the near heap shares one 64 KB segment with the stack and
the globals, so a 64000-byte buffer would not fit in it, and the demos want several of them.

## 2. Types and the 16-bit `int`

On these compilers `int` is **16 bits**: -32768..32767. That has consequences:

* `y * 320` overflows for `y >= 103`, because 103 * 320 = 32960. That is why `vga_put_pixel`
  uses unsigned arithmetic, and why it casts to `unsigned int` before shifting.
* `long int` is 32 bits, and is used where a value can exceed 16 bits (BIOS clock ticks).
* `byte_t` is `unsigned char`, one byte. Colours and palette entries use it.

### Multiplying by shifting

`y * 320` is `y * 256 + y * 64`, which is `(y << 8) + (y << 6)`. On an 8086 a 16-bit multiply
takes over 100 cycles, and shifts and adds are cheaper (a shift by 8 can even be a plain byte
move), so this was a standard trick for a pixel address:

```c
VGA_BUFFER[x + ((unsigned int)y << 8) + ((unsigned int)y << 6)] = c;
```

## 3. Talking to hardware: ports

Besides memory, the CPU has a separate 16-bit **I/O port** address space. The VGA card is
programmed through ports:

| Port  | Purpose                                                |
|-------|--------------------------------------------------------|
| `3C7` | DAC read index: select a colour to read.              |
| `3C8` | DAC write index: select a colour to write.            |
| `3C9` | DAC data: read or write R, then G, then B.            |
| `3DA` | Input status; bit 3 is set during vertical retrace.   |

`outportb(port, value)` writes a byte and `inportb(port)` reads one. For example, to set
colour 5 to a mid grey:

```c
outportb(0x03C8, 5);   /* select colour 5 */
outportb(0x03C9, 32);  /* R */
outportb(0x03C9, 32);  /* G */
outportb(0x03C9, 32);  /* B: the index advances by itself after the third write */
```

## 4. Inline assembly

`vga.c` has a few `asm { ... }` blocks. The compiler assembles them into the function. Here is
the one that clears the drawing buffer, with a comment on each line (the real source has none):

```c
asm {
	PUSH DS                          ; save the data segment
	XOR AX, AX                       ; AX = 0 (the value to store)
	LES DI, DWORD PTR [VGA_BUFFER]   ; ES:DI = far pointer to the buffer
	MOV CX, VGA_BUFFWORDS            ; CX = 32000 words
	REP STOSW                        ; repeat CX times: ES:[DI] = AX; DI += 2
	POP DS                           ; restore the data segment
};
```

The registers involved:

| Register  | Role here                                                     |
|-----------|---------------------------------------------------------------|
| `AX`      | The value stored by `STOSW`. `XOR AX, AX` is the fast way to load 0. |
| `CX`      | The repeat count for `REP`.                                    |
| `ES:DI`   | The destination (extra segment and destination index).         |
| `DS:SI`   | The source (data segment and source index), used when copying. |

* `LES DI, ptr` loads a far pointer into `ES:DI` in one instruction. `LDS SI, ptr` does the
  same for `DS:SI`. Because `LDS` overwrites `DS`, which the C code needs, the blocks that use
  it save and restore `DS` with `PUSH DS` / `POP DS`.
* `REP STOSW` fills memory with `AX`, stepping `DI` upwards (the direction flag is clear by
  default). `REP MOVSW` copies from `DS:SI` to `ES:DI`. Both work on 16-bit words, which is
  why the count is 32000 and not 64000: they move two bytes at a time.
* `INT 10h` calls the BIOS video service. `MOV AX, 0013H` / `INT 10H` enters mode 13h,
  and `MOV AX, 0003H` returns to text mode.

The same operations in C are `_fmemset(VGA_BUFFER, 0, VGA_BUFFSIZE)` and `_fmemcpy(...)`. They are
left in the source as comments next to each block, so you can compare them.

## 5. Two compilers, one source

The sources use Borland names (`asm`, `outportb`, `farmalloc`, `biostime`). To build them
with Open Watcom, `compat/watcom/alloc.h` maps each name to its Watcom equivalent
(`_asm`, `outp`, `_fmalloc`, ...) and stands in for Borland's `<alloc.h>`, which `vga.c` and
`fade.c` include and Open Watcom doesn't have. Building with Turbo C++ uses the real header and needs no shim.

## Further reading

The Intel 8086 manual and Ralf Brown's interrupt list are the references for the
instructions and the BIOS calls. For the VGA registers, look for the PC-GPE
(PC Game Programmer's Encyclopedia).

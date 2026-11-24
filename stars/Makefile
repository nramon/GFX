CFLAGS=-I..\vga

stars.exe: stars.obj ..\vga\vga.obj
	tcc stars.obj ..\vga\vga.obj

stars.obj: stars.c stars.h

..\vga\vga.obj: ..\vga\vga.c ..\vga\vga.h
	tcc -c -n..\vga ..\vga\vga.c

clean:
	-del *.obj
	-del *.exe

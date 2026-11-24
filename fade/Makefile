CFLAGS=-I..\vga

fade.exe: fade.obj ..\vga\vga.obj
	tcc fade.obj ..\vga\vga.obj

fade.obj: fade.c fade.h

..\vga\vga.obj: ..\vga\vga.c ..\vga\vga.h
	tcc -c -n..\vga ..\vga\vga.c

clean:
	-del *.obj
	-del *.exe

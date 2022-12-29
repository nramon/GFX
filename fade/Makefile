CFLAGS=-I../vga

fade.exe: fade.obj
	tcc fade.obj ../vga/vga.obj

fade.obj: fade.c fade.h

clean:
	del *.obj
	del *.exe

CFLAGS=-I..\vga

stars.exe: stars.obj
	tcc stars.obj ..\vga\vga.obj

stars.obj: stars.c stars.h

clean:
	del *.obj
	del *.exe

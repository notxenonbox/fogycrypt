fogyi.exe: fogyi.o windres.o
	x86_64-w64-mingw32-gcc -o fogyi.exe -mwindows fogyi.o windres.o -lshlwapi

fogyi.o: src.c key.h
	x86_64-w64-mingw32-gcc -c -o fogyi.o -O2 src.c

windres.o: resources.rc manifest.xml
	x86_64-w64-mingw32-windres -o windres.o resources.rc

key.h: key.sample.h
	cp key.sample.h key.h

.phony: clean

clean:
	rm -f *.exe *.o

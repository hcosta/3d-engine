# mingw32-make run clean
# flag -g is for gdb debugger purposes

build:
	gcc -Wfatal-errors -g -std=gnu99 ./src/*.c -I"C:/MinGW/libsdl/include" -L"C:/MinGW/libsdl/lib" -lmingw32 -lSDL2main -lSDL2 -lm -o  bin\engine.exe  && bin\engine.exe

run:
	bin\engine.exe

clean:
	del bin\engine.exe
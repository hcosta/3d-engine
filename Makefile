# mingw32-make
# mingw32-make run
# mingw32-make clean

build:
	gcc \
	-Wfatal-errors \
	-std=c99 \
	./src/*.c \
	-I"C:\MinGW\libsdl\include" \
	-L"C:\MinGW\libsdl\lib" \
	-lmingw32 \
	-lSDL2main \
	-lSDL2 \
	-o bin\renderer.exe

run:
	bin\renderer.exe

clean:
	del bin\renderer.exe
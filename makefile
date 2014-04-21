CXX = g++

all: pixl

cairosdl.o: cairosdl.c
	$(CXX) $< -c -o $@ `pkg-config --cflags cairo`

main.o: main.cpp
	$(CXX) $< -c -o $@ `sdl-config --cflags` `pkg-config --cflags pangocairo fontconfig librsvg-2.0`

pixl: main.o cairosdl.o
	$(CXX) $^ -o $@ -O3 -ffast-math -lGL -lGLU -lSDL_ttf `sdl-config --libs` `pkg-config --libs glew pangocairo pangoft2 fontconfig librsvg-2.0`

clean:
	rm *.o pixl

test: pixl
	./pixl

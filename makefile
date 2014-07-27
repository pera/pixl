CXX = g++ -O3

all: pixl

cairosdl.o: cairosdl.c
	$(CXX) $< -c -o $@ `pkg-config --cflags cairo`

app.o: app.cc
	$(CXX) $< -c -o $@ `sdl-config --cflags` `pkg-config --cflags pangocairo fontconfig librsvg-2.0`

filesystem.o: filesystem.cc
	$(CXX) $< -c -o $@

graphics.o: graphics.cc
	$(CXX) $< -c -o $@ `sdl-config --cflags` `pkg-config --cflags pangocairo fontconfig librsvg-2.0`

test.o: test.cc
	$(CXX) $< -c -o $@ `sdl-config --cflags` `pkg-config --cflags pangocairo fontconfig librsvg-2.0`

pixl: test.o cairosdl.o app.o filesystem.o graphics.o
	$(CXX) $^ -o $@ -O3 -ffast-math -lGL -lGLU -lSDL_ttf `sdl-config --libs` -lSDL_image `pkg-config --libs glew pangocairo pangoft2 fontconfig librsvg-2.0`

clean:
	rm *.o pixl

test: pixl
	./pixl

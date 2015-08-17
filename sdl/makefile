default: all
.PHONY: default all clean

CC=gcc
CCFLAGS=-lSDL -lSDLmain
IMAGE_CCFLAGS=-lSDL_image

ALL_OBJECTS=sdlhello blittest sdl_image_test

sdlhello: sdlhello.c
	$(CC) $< -o $@ $(CCFLAGS)
blittest: blittest.c
	$(CC) $< -o $@ $(CCFLAGS)
sdl_image_test: sdl_image_test.c
	$(CC) $< -o $@ $(CCFLAGS) $(IMAGE_CCFLAGS)

all: $(ALL_OBJECTS)
clean:
	-rm $(ALL_OBJECTS)


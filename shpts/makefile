default: all
.PHONY: default all clean real_clean

CC=gcc
CCFLAGS=
SDL_CCFLAGS=-lSDL

CROSS_WIN32_CC=i586-mingw32msvc-gcc
CROSS_WIN32_CC_FLAGS=
CROSS_WIN32_CC_SDL_FLAGS=-lmingw32 -lSDLmain -lSDL

ALL_OBJECTS=read_header read_image_header decode_shp_ts show_shp_ts show_shp_ts.exe
OTHER_FILES=frame.out

read_header: read_header.c
	$(CC) $< -o $@ $(CCFLAGS)
read_image_header: read_image_header.c
	$(CC) $< -o $@ $(CCFLAGS)
decode_shp_ts: decode_shp_ts.c
	$(CC) $< -o $@ $(CCFLAGS)
show_shp_ts: show_shp_ts.c
	$(CC) $< -o $@ $(CCFLAGS) $(SDL_CCFLAGS)
show_shp_ts.exe: show_shp_ts.c
	$(CROSS_WIN32_CC) $< -o $@ $(CROSS_WIN32_CC_FLAGS) $(CROSS_WIN32_CC_SDL_FLAGS)

all: $(ALL_OBJECTS)
clean:
	-rm $(ALL_OBJECTS)
real_clean: clean
	-rm $(OTHER_FILES)


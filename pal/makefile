default: all
.PHONY: default all clean

CC=gcc
CCFLAGS=-lSDL

ALL_OBJECTS=showpal

showpal: showpal.c
	$(CC) $< -o $@ $(CCFLAGS)

all: $(ALL_OBJECTS)
clean:
	-rm $(ALL_OBJECTS)


OBJS = src/8080Core.c src/8080Memory.c

all: testr sinv

testr:
	gcc $(OBJS) src/testrom.c -DDEBUG -O3 -g -Wall -o testrom
sinv:
	gcc $(OBJS) src/SpaceInvaders.c -DDEBUG -g -Wall `sdl-config --cflags --libs` -o spaceinvaders

.PHONY: all testr sinv

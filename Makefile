OBJS = src/8080Core.c src/SpaceInvaders.c
#
all:
	gcc $(OBJS) -g -O3 -Wall `sdl-config --cflags --libs` -o spaceinvaders

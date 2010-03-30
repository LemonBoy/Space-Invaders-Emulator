OBJS = src/8080Core.c src/spaceInvaders.c
#
all:
	gcc $(OBJS) -O3 -Wall `sdl-config --cflags --libs` -o SpaceInvaders.exe

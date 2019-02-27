#include <stdio.h>
#include "8080Core.h"
#include <SDL/SDL.h>

SDL_Surface * screen;
SDL_Event ev;

uint8_t dip0 = 0x0f;
uint8_t dip1 = 0x08;
uint8_t dip2 = 0x01;

uint16_t shiftReg = 0x0000;
int shiftOff = 0x0000;

void die(char *err)
{
	printf("FATAL : %s\n", err);
	dumpRegisters();
	exit(-1);
}


uint8_t spaceInvaders_portIn (int port)
{
	switch (port) {
		case 0:
			return dip0;
		case 1:
			return dip1;
		case 2:
			return dip2;
		case 3:
			return shiftReg >> (8 - shiftOff);
		default:
			printf("Read %d\n", port);
			die("Unknown port access");
	}

	return 0x00;
}

void spaceInvaders_portOut (int port, uint8_t value)
{
	switch (port) {
		case 2:
			shiftOff = value&7; break;
		case 3:
			break;
		case 4:
			shiftReg = (shiftReg >> 8) | ((uint16_t)value << 8); break;
		case 5:
			break;
		case 6:
			break;
		default:
			printf("Write %d %d\n", port, value);
			// die("Unknown port access");
	}
}

void spaceInvaders_vblank ()
{
	int vramPtr, b;

	SDL_LockSurface(screen);

	uint8_t *screenPtr = screen->pixels;

	for (vramPtr = 0; vramPtr < 0x4000 - 0x2400; vramPtr++) {
		for (b=0;b<8;b++) {
			*screenPtr = ((readByte(0x2400 + vramPtr) >> b)&1) ? 0xFF : 0x00;
			screenPtr++;
		}
	}

	SDL_UnlockSurface(screen);

	SDL_Flip(screen);
}

void spaceInvaders_update_input ()
{
	dip0 = 0x0e;
	dip1 = 0x08;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
			case SDL_KEYUP:
				switch(ev.key.keysym.sym) {
					case SDLK_LEFT:
						dip1 &= ~(1 << 5); break;
					case SDLK_RIGHT:
						dip1 &= ~(1 << 6); break;
					default:
						break;
				}
				break;
			case SDL_KEYDOWN:
				switch(ev.key.keysym.sym) {
					case SDLK_LEFT:
						dip1 |= (1 << 5); break;
					case SDLK_RIGHT:
						dip1 |= (1 << 6); break;
					case SDLK_c:
						dip1 |= (1 << 0); break;
					case SDLK_x:
						dip1 |= (1 << 2); break;
					case SDLK_z:
						dip1 |= (1 << 4); break;
					default:
						break;
				} break;
			case SDL_QUIT:
				exit(0);
				break;
			default:
				break;
		}
	}
}

int main(int argc, char *argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
		printf("Cannot initialize SDL\n");
		exit(0);
	}

	atexit(SDL_Quit);

	// SDL_EnableKeyRepeat(0, 0);

	screen = SDL_SetVideoMode(256, 224, 8, SDL_DOUBLEBUF);

	SDL_WM_SetCaption("OMGALIENZATEMYLEM0N", NULL);

	reset8080(0x0001);

	uint8_t *buf = registerBank(0x0000, 4 * 0x0800, NULL, 1);
	char *bankName[] = {"invaders.h", "invaders.g", "invaders.f", "invaders.e"};
	// char *bankName[] = {"se_test_716.bin", "invaders.g", "invaders.f", "invaders.e"};
	int i;

	for (i = 0; i < 4; i++) {
		FILE *fp = fopen(bankName[i], "rb");
		if (!fp) {
			return 0;
		}
		if (fread(buf + (i * 0x0800), 1, 0x0800, fp) != 0x0800) {
			return 0;
		}
		fclose(fp);
	}

	buf[0] = 0xc3;

	registerBank(0x2000, 0x2000, NULL, 0);
	// registerMirror(0x4000, 0x2000, 0x0000);
	// registerMirror(0x6000, 0x2000, 0x2000);

	e8080.portIn = spaceInvaders_portIn;
	e8080.portOut = spaceInvaders_portOut;

	while (!e8080.halt)
	{
		emulate8080(28527);
		causeInt(8);
		spaceInvaders_vblank();
		emulate8080(4839);
		causeInt(16);
		spaceInvaders_update_input();
		SDL_Delay(15);
	}

	return 1;
}

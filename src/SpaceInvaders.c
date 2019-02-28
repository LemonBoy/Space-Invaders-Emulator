#include <stdio.h>
#include "8080Core.h"
#include <SDL2/SDL.h>

static SDL_Window *win;
static SDL_Renderer *renderer;
static SDL_Texture *screentex;
static SDL_Event ev;
static uint8_t screen_buf[256*224*4];

static uint8_t dip0 = 0x0f;
static uint8_t dip1 = 0x08;
static uint8_t dip2 = 0x01;

static uint16_t shiftReg = 0x0000;
static int shiftOff = 0;

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
			printf("Port read %d\n", port);
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
			printf("Port write %d %d\n", port, value);
	}
}

void spaceInvaders_vblank ()
{
	uint16_t vram_base = 0x2400;
	uint32_t *screenPtr = screen_buf;
	int i;

	while (vram_base < 0x4000) {
		uint8_t b = readByte(vram_base);

		*screenPtr++ = ((b >> 0)&1) ?  0xFFFFFFFF : 0xFF000000;
		*screenPtr++ = ((b >> 1)&1) ?  0xFFFFFFFF : 0xFF000000;
		*screenPtr++ = ((b >> 2)&1) ?  0xFFFFFFFF : 0xFF000000;
		*screenPtr++ = ((b >> 3)&1) ?  0xFFFFFFFF : 0xFF000000;
		*screenPtr++ = ((b >> 4)&1) ?  0xFFFFFFFF : 0xFF000000;
		*screenPtr++ = ((b >> 5)&1) ?  0xFFFFFFFF : 0xFF000000;
		*screenPtr++ = ((b >> 6)&1) ?  0xFFFFFFFF : 0xFF000000;
		*screenPtr++ = ((b >> 7)&1) ?  0xFFFFFFFF : 0xFF000000;

		vram_base += 1;
	}

	SDL_UpdateTexture(screentex, NULL, screen_buf, 256 * 4);
}

void spaceInvaders_update_input ()
{
	dip0  = 0x0E;
	dip1 &= 0xE0;

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
		return 0;
	}

	atexit(SDL_Quit);

	if (SDL_CreateWindowAndRenderer(256, 256, SDL_WINDOW_OPENGL, &win,
					&renderer)) {
		printf("Cannot create a new window\n");
		return 0;
	}

	screentex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
				      SDL_TEXTUREACCESS_STREAMING, 256, 224);

	reset8080(0x0001);

	uint8_t *buf = registerBank(0x0000, 4 * 0x0800, NULL, 1);
	char *bankName[] = {"invaders.h", "invaders.g", "invaders.f", "invaders.e"};
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

	e8080.portIn = spaceInvaders_portIn;
	e8080.portOut = spaceInvaders_portOut;

	SDL_Rect dest_rect = { 0, 0, 256, 224 };

	while (!e8080.halt) {
		emulate8080(17066);
		causeInt(8);
		spaceInvaders_vblank();
		emulate8080(17066);
		causeInt(16);
		spaceInvaders_update_input();
		SDL_Delay(15);

		SDL_RenderClear(renderer);
		SDL_RenderCopyEx(renderer, screentex, NULL, &dest_rect, 270, NULL, 0);
		SDL_RenderPresent(renderer);
	}

	return 1;
}

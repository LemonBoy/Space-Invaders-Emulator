#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "8080Core.h"

typedef struct Mem {
	struct Mem *next;
	uint32_t base;
	uint32_t size;
	uint32_t mirror;
	uint8_t *buf;
	int is_mirror;
	int is_readonly;
} Mem;

void registerMirror(uint32_t base, uint32_t size, uint32_t to)
{
	Mem *new = malloc(sizeof(Mem));
	new->next = NULL;
	new->base = base;
	new->size = size;
	new->mirror = to;
	new->buf = NULL;
	new->is_mirror = 1;
	new->is_readonly = 0;

	new->next = e8080.ram;
	e8080.ram = new;
}

uint8_t *registerBank(uint32_t base, uint32_t size, uint8_t *buf, int ro)
{
	Mem *new = malloc(sizeof(Mem));
	new->next = NULL;
	new->base = base;
	new->size = size;
	new->mirror = 0;
	new->is_mirror = 0;
	new->is_readonly = ro;

	if (buf != NULL) new->buf = buf;
	else new->buf = calloc(1, size);

	new->next = e8080.ram;
	e8080.ram = new;

	return new->buf;
}

void writeByte (uint8_t value, uint16_t offset)
{
	Mem *bank = e8080.ram;

	while (bank) {
		if (bank->is_mirror &&
		    offset >= bank->base && offset < bank->base + bank->size) {
			// Translate to its base address
			uint16_t old = offset;
			int d = bank->mirror != 0 ? bank->mirror : bank->size;
			offset = (offset % d) + bank->mirror;
			// Re-scan
			bank = e8080.ram;
			continue;
		}
		else if (!bank->is_mirror &&
			 offset >= bank->base && offset < bank->base + bank->size) {
			if (!bank->is_readonly) {
				bank->buf[offset - bank->base] = value;
			}
			return;
		}
		bank = bank->next;
	}

	printf("Illegal write @ %04x\n", offset);
	dumpRegisters();
	fflush(stdout);
	assert(0);
}

uint8_t readByte (uint16_t offset)
{
	Mem *bank = e8080.ram;

	while (bank) {
		if (bank->is_mirror &&
		    offset >= bank->base && offset < bank->base + bank->size) {
			// Translate to its base address
			uint16_t old = offset;
			int d = bank->mirror != 0 ? bank->mirror : bank->size;
			offset = (offset % d) + bank->mirror;
			// Re-scan
			bank = e8080.ram;
			continue;
		}
		else if (!bank->is_mirror &&
			 offset >= bank->base && offset < bank->base + bank->size) {
			return bank->buf[offset - bank->base];
		}
		bank = bank->next;
	}

	printf("Illegal read @ %04x\n", offset);
	dumpRegisters();
	fflush(stdout);
	assert(0);
}

void stackPush (uint16_t value)
{
	e8080.SP -= 2;
	writeByte((value >> 8) & 0xFF, e8080.SP + 1);
	writeByte((value >> 0) & 0xFF, e8080.SP + 0);
}

uint16_t stackPop ()
{
	uint8_t lo = readByte(e8080.SP + 0);
	uint8_t hi = readByte(e8080.SP + 1);
	e8080.SP += 2;
	return hi << 8 | lo;
}

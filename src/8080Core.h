#ifndef E8080CORE_H
#define E8080CORE_H

#include <stdint.h>

#include "8080Memory.h"

struct {
	/* Processor frequency MhZ */
	int clock;
	/* Halt flag */
	uint8_t halt;
	/* 8080 registers */
	union {
		struct {
			uint8_t C;
			uint8_t B;
			uint8_t E;
			uint8_t D;
			uint8_t L;
			uint8_t H;
			uint8_t F;
			uint8_t A;
		};
		struct {
			uint16_t BC;
			uint16_t DE;
			uint16_t HL;
			uint16_t AF;
		};
	};
	/* Program Counter & Stack Pointer */
	uint16_t PC;
	uint16_t SP;
	/* Working ram */
	Mem *ram;
	/* Interrupts enabled */
	uint8_t IE;
	uint8_t iPending;
	/* Ports IO */
	uint8_t (* portIn)(int port);
	void (*portOut)(int port, uint8_t value);
} e8080;

#define FLAG_SIGN   (1 << 7)
#define FLAG_ZERO   (1 << 6)
#define FLAG_ACARRY (1 << 4)
#define FLAG_PARITY (1 << 2)
#define FLAG_CARRY  (1 << 0)

void reset8080(uint16_t pc);
int emulate8080(int cycles);
void causeInt(uint16_t address);
void dumpRegisters();

#endif

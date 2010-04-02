#include <stdio.h>
#include <string.h>
#include "8080Core.h"
#include "8080Opcodes.h"

void dumpRegisters ()
{
	printf("A : %02X F : %02X\nB : %02x C : %02x\nD : %02x E : %02x\nH : %02x L : %02x\nPC : %04x SP : %04x\n",
		e8080.A, e8080.F, e8080.B, e8080.C, e8080.D, e8080.E, e8080.H, e8080.L, e8080.PC, e8080.SP);
}

void causeInt (int address)
{
	if (e8080.IE) {
		stackPush(e8080.PC);
		e8080.PC = (address & 0xFFFF);
	}
}

int emulate8080 (int cycles)
{
	int cyclesDone = 0;
	
	while (cyclesDone < cycles) 
	{
		if (e8080.halt) {
			printf("Halted!\n");
			break;
		}

		u8 opcode = fetch8();
		
		opTbl[opcode].execute (opcode);
		cyclesDone += opTbl[opcode].cycles;
	}
	
	return cyclesDone;
}

void reset8080 ()
{
	memset(&e8080, 0, sizeof e8080);
	e8080.SP = 0xF000;
}

int initalize8080 ()
{
	FILE * romBank;
	char *bankName[] = {"invaders.h", "invaders.g", "invaders.f", "invaders.e"};
	int c;
	
	printf("Resetting the 8080...\n");
	
	reset8080();
	
	for (c = 0; c < 4; c++) {
		romBank = fopen(bankName[c], "rb");
		if (!romBank) {
			return 0;
		}
		if (fread(&e8080.ram[0x0800 * c], 1, 0x0800, romBank) != 0x0800) {
			return 0;
		}
		fclose(romBank);
	}
	
	printf("Loaded the 4 rom banks...\n");
	
	return 1;
}

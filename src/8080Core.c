#include <stdio.h>
#include <string.h>
#include "8080Core.h"
#include "8080Opcodes.h"

void causeInt (int address)
{
	if (e8080.IE) {
		stackPush(e8080.PC);
		e8080.PC = (address & 0xFFFF);
	}
}

#ifdef DEBUG
static const char* lut_mnemonic[0x100]={
	"nop",     "lxi b,#", "stax b",  "inx b",   "inr b",   "dcr b",   "mvi b,#", "rlc",     "ill",     "dad b",   "ldax b",  "dcx b",   "inr c",   "dcr c",   "mvi c,#", "rrc",
	"ill",     "lxi d,#", "stax d",  "inx d",   "inr d",   "dcr d",   "mvi d,#", "ral",     "ill",     "dad d",   "ldax d",  "dcx d",   "inr e",   "dcr e",   "mvi e,#", "rar",
	"ill",     "lxi h,#", "shld",    "inx h",   "inr h",   "dcr h",   "mvi h,#", "daa",     "ill",     "dad h",   "lhld",    "dcx h",   "inr l",   "dcr l",   "mvi l,#", "cma",
	"ill",     "lxi sp,#","sta $",   "inx sp",  "inr M",   "dcr M",   "mvi M,#", "stc",     "ill",     "dad sp",  "lda $",   "dcx sp",  "inr a",   "dcr a",   "mvi a,#", "cmc",
	"mov b,b", "mov b,c", "mov b,d", "mov b,e", "mov b,h", "mov b,l", "mov b,M", "mov b,a", "mov c,b", "mov c,c", "mov c,d", "mov c,e", "mov c,h", "mov c,l", "mov c,M", "mov c,a",
	"mov d,b", "mov d,c", "mov d,d", "mov d,e", "mov d,h", "mov d,l", "mov d,M", "mov d,a", "mov e,b", "mov e,c", "mov e,d", "mov e,e", "mov e,h", "mov e,l", "mov e,M", "mov e,a",
	"mov h,b", "mov h,c", "mov h,d", "mov h,e", "mov h,h", "mov h,l", "mov h,M", "mov h,a", "mov l,b", "mov l,c", "mov l,d", "mov l,e", "mov l,h", "mov l,l", "mov l,M", "mov l,a",
	"mov M,b", "mov M,c", "mov M,d", "mov M,e", "mov M,h", "mov M,l", "hlt",     "mov M,a", "mov a,b", "mov a,c", "mov a,d", "mov a,e", "mov a,h", "mov a,l", "mov a,M", "mov a,a",
	"add b",   "add c",   "add d",   "add e",   "add h",   "add l",   "add M",   "add a",   "adc b",   "adc c",   "adc d",   "adc e",   "adc h",   "adc l",   "adc M",   "adc a",
	"sub b",   "sub c",   "sub d",   "sub e",   "sub h",   "sub l",   "sub M",   "sub a",   "sbb b",   "sbb c",   "sbb d",   "sbb e",   "sbb h",   "sbb l",   "sbb M",   "sbb a",
	"ana b",   "ana c",   "ana d",   "ana e",   "ana h",   "ana l",   "ana M",   "ana a",   "xra b",   "xra c",   "xra d",   "xra e",   "xra h",   "xra l",   "xra M",   "xra a",
	"ora b",   "ora c",   "ora d",   "ora e",   "ora h",   "ora l",   "ora M",   "ora a",   "cmp b",   "cmp c",   "cmp d",   "cmp e",   "cmp h",   "cmp l",   "cmp M",   "cmp a",
	"rnz",     "pop b",   "jnz $",   "jmp $",   "cnz $",   "push b",  "adi #",   "rst 0",   "rz",      "ret",     "jz $",    "ill",     "cz $",    "call $",  "aci #",   "rst 1",
	"rnc",     "pop d",   "jnc $",   "out p",   "cnc $",   "push d",  "sui #",   "rst 2",   "rc",      "ill",     "jc $",    "in p",    "cc $",    "ill",     "sbi #",   "rst 3",
	"rpo",     "pop h",   "jpo $",   "xthl",    "cpo $",   "push h",  "ani #",   "rst 4",   "rpe",     "pchl",    "jpe $",   "xchg",    "cpe $",   "ill",     "xri #",   "rst 5",
	"rp",      "pop psw", "jp $",    "di",      "cp $",    "push psw","ori #",   "rst 6",   "rm",      "sphl",    "jm $",    "ei",      "cm $",    "ill",     "cpi #",   "rst 7"
};
#endif

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
#ifdef DEBUG		
		printf("[%04x] %s\n", e8080.PC, lut_mnemonic[opcode]);	
#endif
		
		opTbl[opcode].execute (opcode);
		cyclesDone += opTbl[opcode].cycles;
	}
	
	return cyclesDone;
}

void reset8080 ()
{
	int c;
	memset(&e8080, 0, sizeof e8080);
	e8080.SP = 0xF000;
	e8080.ram = (ramBank *)&wram8080;
	for (c = 0; c < 4; c++) {
		if (e8080.ram[c].flag == FLAG_UNUSED)
			continue;
		if (e8080.ram[c].flag == FLAG_MIRROR)
			continue;					
		if (!e8080.ram[c].ptr)
			e8080.ram[c].ptr = malloc(e8080.ram[c].size);
		if (!e8080.ram[c].flag != FLAG_ROM)
			memset(e8080.ram[c].ptr, 0, e8080.ram[c].size);
	}
}

int initalize8080 ()
{
	FILE * romBank;
	char *bankName[] = {"invaders.h", "invaders.g", "invaders.f", "invaders.e"};
	int c;
	
	printf("Resetting the 8080...\n");
	
	reset8080();
	
	u8 *bankPtr = NULL;
	
	for (c = 0; c < 4; c++) {
		if (e8080.ram[c].flag == FLAG_ROM) {
			printf("ROM @ %#x (%#x) %p\n", e8080.ram[c].start, e8080.ram[c].size, e8080.ram[c].ptr);
			bankPtr = e8080.ram[c].ptr;
			break;
		}
	}
			
	if (!bankPtr)
		die("No rom bank defined");
			
	for (c = 0; c < 4; c++) {
		romBank = fopen(bankName[c], "rb");
		if (!romBank) {
			return 0;
		}
		if (fread(&bankPtr[0x0800 * c], 1, 0x0800, romBank) != 0x0800) {
			return 0;
		}
		fclose(romBank);
	}
	
	printf("Loaded the 4 rom banks...\n");
	
	return 1;
}

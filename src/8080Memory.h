#ifndef E8080MEMORY_H
#define E8080MEMORY_H

#include "8080Core.h"

#define WHITIN(x,y,z) ((x >= y) && (x <= z))

inline void writeByte (u8 value, u16 offset)
{
	int c;
	
	for (c = 0; c < 4; c++) {
		if (!WHITIN(offset, e8080.ram[c].start, e8080.ram[c].start + e8080.ram[c].size))
			continue;		
		if (e8080.ram[c].flag == FLAG_MIRROR) {
			offset -= e8080.ram[c].start;
			offset += e8080.ram[c].mirror;
			break;
		}
	}	
	
	for (c = 0; c < 8; c++) {
		if (e8080.ram[c].flag == FLAG_UNUSED)
			continue;
		if (!WHITIN(offset, e8080.ram[c].start, e8080.ram[c].start + e8080.ram[c].size))
			continue;
		if (e8080.ram[c].flag == FLAG_ROM)
			die("Write to ROM region");
#ifdef DEBUG	
		printf("Write %#x @ %#x\n", value, offset);
#endif	
		e8080.ram[c].ptr[offset - e8080.ram[c].start] = value;
		return;
	}
	
	die("Write out of bounds!");
}

inline u8 readByte (u16 offset)
{
	int c;
	
	for (c = 0; c < 4; c++) {
		if (!WHITIN(offset, e8080.ram[c].start, e8080.ram[c].start + e8080.ram[c].size))
			continue;		
		if (e8080.ram[c].flag == FLAG_MIRROR) {
			offset -= e8080.ram[c].start;
			offset += e8080.ram[c].mirror;
			break;
		}
	}
	
	for (c = 0; c < 4; c++) {
		if (e8080.ram[c].flag == FLAG_UNUSED)
			continue;
		if (!WHITIN(offset, e8080.ram[c].start, e8080.ram[c].start + e8080.ram[c].size))
			continue;
#ifdef DEBUG	
		printf("Read @ %#x\n", value, offset);
#endif	
		return e8080.ram[c].ptr[offset - e8080.ram[c].start];
	}
	
	die("Read out of bounds!");
	
	return 0xFF;
}

static u16 stack[0xFFFF];
static int stackPtr = 0;

inline void stackPush (u16 value)
{
	if (e8080.SP < 2) {
		die("Stack pointer exhausted");
	}
	/*writeByte((value >> 8) & 0xFF, --e8080.SP);
	writeByte((value >> 0) & 0xFF, --e8080.SP);*/
	stack[stackPtr++] = value;
}

inline u16 stackPop ()
{
	/*u16 r  =  readByte(e8080.SP++);
	    r |= (readByte(e8080.SP++) << 8);
	return r;*/
	return stack[--stackPtr];
}


#endif

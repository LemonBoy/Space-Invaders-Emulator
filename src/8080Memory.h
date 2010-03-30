#ifndef E8080MEMORY_H
#define E8080MEMORY_H

#include "8080Core.h"

inline char *getSection (u16 offset)
{
	if (offset < 0x2000) {
		return "Rom";
	} else if (offset >= 0x2000 && offset < 0x2400) {
		return "Ram";
	} else if (offset >= 0x2400) {
		return "Video ram";
	}
	
	return "Unknown";
}

inline void writeByte (u8 value, u16 offset)
{
	if (offset > 0x4000) {
		offset -= 0x2000;
	}
	if ((offset > sizeof(e8080.ram)) || (offset < 0x2000)) {
		return;
	}
#ifdef DEBUG	
	printf("Write %#X at %#X (%s)\n", value, offset, getSection(offset));
#endif
	e8080.ram[offset] = value;
}

inline u8 readByte (u16 offset)
{
	if (offset > 0x4000) {
		offset -= 0x2000;
	}
#ifdef DEBUG	
	printf("Read at %#X (%s)\n", offset, getSection(offset));
#endif
	return e8080.ram[offset];
}

inline void stackPush (u16 value)
{
	writeByte((value >> 8) & 0xFF, --e8080.SP);
	writeByte((value >> 0) & 0xFF, --e8080.SP);
}

inline u16 stackPop ()
{
	u16 r  =  readByte(e8080.SP++);
	    r |= (readByte(e8080.SP++) << 8);
	return r;
}


#endif

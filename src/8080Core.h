#ifndef E8080CORE_H
#define E8080CORE_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define CYCLES_PER_FRAME(fps) (2000000 / (2*fps))

struct {
	/* Processor frequency MhZ */
	int clock;
	/* Halt flag */
	u8 halt;
	/* 8080 registers */
	union {
		struct {
			u8 C;
			u8 B;
			u8 E;
			u8 D;
			u8 L;
			u8 H;
			u8 F;
			u8 A;
		};
		struct {
			u16 BC;
			u16 DE;
			u16 HL;
			u16 AF;
		};
	};
	/* Program Counter & Stack Pointer */
	u16 PC;
	u16 SP;
	/* Working ram */
	u8 ram[0x4000];
	/* Interrupts enabled */
	u8 IE;
	u8 iPending;
	/* Ports IO */
	u8   (* portIn) (int port);
	void (*portOut) (int port, u8 value);
} e8080;

#define FLAG_SIGN 	(1 << 7)
#define FLAG_ZERO 	(1 << 6)
#define FLAG_ACARRY (1 << 4)
#define FLAG_PARITY (1 << 2)
#define FLAG_CARRY 	(1 << 0)

int initalize8080 ();
void reset8080 ();
int emulate8080 (int cycles);
void causeInt (int address);

#endif

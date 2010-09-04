#ifndef E8080CORE_H
#define E8080CORE_H

typedef unsigned char 	u8;
typedef unsigned short 	u16;
typedef unsigned int 	u32;

#define BIT(x)		(1 << x)

#define FLAG_UNUSED 0x00
#define FLAG_USED	BIT(0)
#define FLAG_ROM	BIT(1)
#define FLAG_MIRROR BIT(2)

typedef struct {
	u32 start;
	u32 size;
	u32 mirror;
	u8  flag;
	u8 *ptr;
} ramBank;

extern ramBank wram8080[4];

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
	ramBank *ram;
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

static void dumpRegisters ()
{
	printf("A : %02X F : %02X\nB : %02x C : %02x\nD : %02x E : %02x\nH : %02x L : %02x\nPC : %04x SP : %04x\n",
		e8080.A, e8080.F, e8080.B, e8080.C, e8080.D, e8080.E, e8080.H, e8080.L, e8080.PC, e8080.SP);
}

extern void exit(int);

static inline void die(char *err) 
{
	printf("FATAL : %s\n", err);
	dumpRegisters();
	exit(-1);
}

int initalize8080 ();
void reset8080 ();
int emulate8080 (int cycles);
void causeInt (int address);
u8 readByte (u16 offset);
void writeByte (u8 value, u16 offset);

#endif

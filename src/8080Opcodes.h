#ifndef E8080OPCODES_H
#define E8080OPCODES_H

#include "8080Core.h"
#include "8080Memory.h"

/*
 * Pipeline fetching
*/

u8 fetch8 ()
{
	return readByte(e8080.PC++);
}

u16 fetch16 ()
{
	u16 r = fetch8() | fetch8() << 8 ;
	return r;
}

inline void setFlags (u32 reg, int fMask)
{
	//int t;
	//for (t=0;t<8;t++) { printf("%02x ", e8080.F & (1 >> (7 - t))); } printf("\n");

	e8080.F &= ~(fMask);
	
	//for (t=0;t<8;t++) { printf("%02x ", e8080.F & (1 >> (7 - t))); } printf("\n");
	
	if (fMask & FLAG_SIGN)
	{ 
		if (reg & (1 << 7)) {
			e8080.F |= FLAG_SIGN;
		}
	}
	
	if (fMask & FLAG_ZERO)
	{ 
		if (!reg) {
			e8080.F |= FLAG_ZERO;
		}
	}
	
	if (fMask & FLAG_CARRY)
	{ 
		if ((reg & 0x100) != 0) {
			e8080.F |= FLAG_CARRY;
		}
	}
	
	if (fMask & FLAG_ACARRY)
	{ 
		if ((e8080.A & 0xf) > (reg & 0xf)) {
			e8080.F |= FLAG_ACARRY;
		}
	}
	
	if (fMask & FLAG_PARITY)
	{ 
		if (!(reg & 0x1)) {
			e8080.F |= FLAG_PARITY;
		}
	}
}

inline void writeReg (u8 value, int index)
{
	switch (index) {
		case 0x00:
			e8080.B = value; break;
		case 0x01:
			e8080.C = value; break;
		case 0x02:
			e8080.D = value; break;
		case 0x03:
			e8080.E = value; break;
		case 0x04:
			e8080.H = value; break;
		case 0x05:
			e8080.L = value; break;
		case 0x06:
			writeByte(value, e8080.HL); break;
		case 0x07:
			e8080.A = value; break;
	}
}

inline u8 readReg (int index)
{
	switch (index) {
		case 0x00:
			return e8080.B; break;
		case 0x01:
			return e8080.C; break;
		case 0x02:
			return e8080.D; break;
		case 0x03:
			return e8080.E; break;
		case 0x04:
			return e8080.H; break;
		case 0x05:
			return e8080.L; break;
		case 0x06:
			return readByte(e8080.HL); break;
		case 0x07:
			return e8080.A; break;
	}
	return 0xFF;
}

inline void CMC (u8 opcode)
{
	e8080.F ^= FLAG_CARRY;
}

inline void STC (u8 opcode)
{
	e8080.F |= FLAG_CARRY;
}

inline void DAA (u8 opcode)
{
	int top4 = (e8080.A >> 4) & 0xF;
	int bot4 = (e8080.A & 0xF);
	
	if ((bot4 > 9) || (e8080.F & FLAG_ACARRY)) {
		setFlags(e8080.A + 6, FLAG_ZERO | FLAG_SIGN | FLAG_PARITY | FLAG_CARRY | FLAG_ACARRY);
		e8080.A += 6;
		top4 = (e8080.A >> 4) & 0xF;
		bot4 = (e8080.A & 0xF);
	}
	
	if ((top4 > 9) || (e8080.F & FLAG_CARRY)) {
		top4 += 6;
		e8080.A = (top4 << 4) | bot4;
	}
}
		

inline void INR (u8 opcode)
{
	int dst = (opcode >> 3) & 0x7;
	u8 tmp = readReg(dst);
	tmp++;
	setFlags(tmp, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY);
	writeReg(tmp, dst);
}

inline void DCR (u8 opcode)
{
	int dst = (opcode >> 3) & 0x7;
	u8 tmp = readReg(dst);
	tmp--;
	setFlags(tmp, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY);
	writeReg(tmp, dst);
}

inline void CMA (u8 opcode)
{
	e8080.A = ~(e8080.A);
}

inline void NOP (u8 opcode)
{
}

inline void MOV (u8 opcode)
{
	int dst = (opcode >> 3) & 0x7;
	int src = (opcode & 0x7);
	u8 srcVal = readReg(src);
	writeReg(srcVal, dst);
}

inline void STAX (u8 opcode)
{
	int src = (opcode >> 4) & 1;
	u16 addr = (!src) ? e8080.BC : e8080.DE;
	writeByte(e8080.A, addr);
}

inline void LDAX (u8 opcode)
{
	int src = (opcode >> 4) & 1;
	u16 addr = (!src) ? e8080.BC : e8080.DE;
	e8080.A = readByte(addr);
}

inline void ADD (u8 opcode)
{
	int src = (opcode & 0x7);
	u8 srcVal = readReg(src);
	setFlags(e8080.A + srcVal, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A += srcVal;
}

inline void ADC (u8 opcode)
{
	int src = (opcode & 0x7);
	u8 srcVal = readReg(src);
	setFlags(e8080.A + srcVal + (e8080.F & FLAG_CARRY), FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A += srcVal + (e8080.F & FLAG_CARRY);
}
	
inline void SUB (u8 opcode)
{
	int src = (opcode & 0x7);
	u8 srcVal = readReg(src);
	setFlags(e8080.A - srcVal, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A -= srcVal;
}

inline void SBB (u8 opcode)
{
	int src = (opcode & 0x7);
	u8 srcVal = readReg(src);
	setFlags(e8080.A - (srcVal + (e8080.F & FLAG_CARRY)), FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A -= (srcVal + (e8080.F & FLAG_CARRY));
}

inline void ANA (u8 opcode)
{
	int src = (opcode & 0x7);
	u8 srcVal = readReg(src);
	setFlags(e8080.A & srcVal, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A &= srcVal;
}

inline void XRA (u8 opcode)
{
	int src = (opcode & 0x7);
	u8 srcVal = readReg(src);
	setFlags(e8080.A ^ srcVal, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A ^= srcVal;
}

inline void ORA (u8 opcode)
{
	int src = (opcode & 0x7);
	u8 srcVal = readReg(src);
	setFlags(e8080.A | srcVal, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A |= srcVal;
}

inline void CMP (u8 opcode)
{
	int src = (opcode & 0x7);
	u8 srcVal = readReg(src);
	setFlags(e8080.A - srcVal, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
}

inline void RLC (u8 opcode)
{
	e8080.F = ~(FLAG_CARRY);
	if (e8080.A & (1 << 7)) {
		e8080.F |= FLAG_CARRY;
	}
	e8080.A = (e8080.A << 1) | (e8080.F & FLAG_CARRY);
}

inline void RRC (u8 opcode)
{
	e8080.F = ~(FLAG_CARRY);
	if (e8080.A & 1) {
		e8080.F |= FLAG_CARRY;
	}
	e8080.A = (e8080.A >> 1) | ((e8080.F & FLAG_CARRY) << 7);
}

inline void RAL (u8 opcode)
{
	u8 c = (e8080.A & (1 << 7));
	e8080.A = (e8080.A << 1) | (e8080.F & FLAG_CARRY);
	e8080.F = c;
}

inline void RAR (u8 opcode)
{
	u8 c = (e8080.A & 1);
	e8080.A = (e8080.A >> 1) | ((e8080.F & FLAG_CARRY) << 7);
	e8080.F = c;
}

inline void PUSH (u8 opcode)
{
	int src = (opcode >> 4) & 3;
	switch (src) {
		case 0x00:
			stackPush(e8080.BC); break;
		case 0x01:
			stackPush(e8080.DE); break;
		case 0x02:
			stackPush(e8080.HL); break;
		case 0x03:
			stackPush(e8080.AF); break;	
		default:
			while (1);
	}
}

inline void POP (u8 opcode)
{
	int src = (opcode >> 4) & 3;
	u16 val = stackPop();
	switch (src) {
		case 0x00:
			e8080.BC = val;
			break;
		case 0x01:
			e8080.DE = val;
			break;
		case 0x02:
			e8080.HL = val;
			break;
		case 0x03:
			e8080.AF = val;
			break;
		default:
			while (1);			
	}
}

inline void DAD (u8 opcode)
{
	int src = (opcode >> 4) & 3;
	u16 rVal = 0;
	switch (src) {
		case 0x00:
			rVal = e8080.BC; break;
		case 0x01:
			rVal = e8080.DE; break;
		case 0x02:
			rVal = e8080.HL; break;
		case 0x03:
			rVal = e8080.AF; break;			
	}
	setFlags(e8080.HL + rVal, FLAG_CARRY);
	e8080.HL += rVal;
}

inline void INX (u8 opcode)
{
	int src = (opcode >> 4) & 3;
	switch (src) {
		case 0x00:
			e8080.BC++;
			break;
		case 0x01:
			e8080.DE++;
			break;
		case 0x02:
			e8080.HL++;
			break;
		case 0x03:
			e8080.SP++; 
			break;
		default:
			while (1);
	}
}

inline void DCX (u8 opcode)
{
	int src = (opcode >> 4) & 3;
	switch (src) {
		case 0x00:
			e8080.BC--;
			break;
		case 0x01: 
			e8080.DE--;
			break;
		case 0x02:
			e8080.HL--;
			break;
		case 0x03:
			e8080.SP--; 
			break;
		default:
			while (1);			
	}
}

inline void XCHG (u8 opcode)
{
	u16 tmp1, tmp2;
	tmp1 = e8080.DE;
	tmp2 = e8080.HL;
	e8080.DE = e8080.HL;
	e8080.HL = e8080.DE;
}

inline void XTHL (u8 opcode)
{
	e8080.H = readByte(e8080.SP);
	e8080.L = readByte(e8080.SP + 1);
}

inline void SPHL (u8 opcode)
{
	e8080.SP = e8080.HL;
}

inline void LXI (u8 opcode)
{
	int dst = (opcode >> 4) & 3;
	switch (dst) {
		case 0x00:
			e8080.BC = fetch16();
			break;
		case 0x01:
			e8080.DE = fetch16();
			break;
		case 0x02:
			e8080.HL = fetch16();
			break;
		case 0x03:
			e8080.SP = fetch16(); 
			break;			
		default:
			while (1);			
	}
}

inline void MVI (u8 opcode)
{
	int dst = (opcode >> 3) & 0x7;
	writeReg(fetch8(), dst);
}

inline void ADI (u8 opcode)
{
	u8 imm = fetch8();
	setFlags(e8080.A + imm, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A += imm;
}

inline void ACI (u8 opcode)
{
	u8 imm = fetch8();
	setFlags(e8080.A + imm + (e8080.F & FLAG_CARRY), FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A += (imm + (e8080.F & FLAG_CARRY));
}

inline void SUI (u8 opcode)
{
	u8 imm = fetch8();
	setFlags(e8080.A - imm, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A -= imm;
}

inline void SBI (u8 opcode)
{
	u8 imm = fetch8();
	setFlags(e8080.A - (imm + (e8080.F & FLAG_CARRY)), FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A -= (imm + (e8080.F & FLAG_CARRY));
}

inline void ANI (u8 opcode)
{
	u8 imm = fetch8();
	setFlags(e8080.A & imm, FLAG_SIGN | FLAG_ZERO | FLAG_PARITY | FLAG_CARRY);
	e8080.A &= imm;
}

inline void XRI (u8 opcode)
{
	u8 imm = fetch8();
	setFlags(e8080.A ^ imm, FLAG_SIGN | FLAG_ZERO | FLAG_PARITY | FLAG_CARRY);
	e8080.A ^= imm;
}

inline void ORI (u8 opcode)
{
	u8 imm = fetch8();
	setFlags(e8080.A | imm, FLAG_SIGN | FLAG_ZERO | FLAG_PARITY | FLAG_CARRY);
	e8080.A |= imm;
}

inline void CPI (u8 opcode)
{
	u8 imm = fetch8();
	setFlags(e8080.A - imm, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A -= imm;
}

inline void STA (u8 opcode)
{
	u16 addr = fetch16();
	writeByte(e8080.A, addr);
}

inline void LDA (u8 opcode)
{
	u16 addr = fetch16();
	e8080.A = readByte(addr);
}

inline void SHLD (u8 opcode)
{
	u16 addr = fetch16();
	writeByte(e8080.H, addr);
	writeByte(e8080.L, addr + 1);
}

inline void LHLD (u8 opcode)
{
	u16 addr = fetch16();
	e8080.H = readByte(addr);
	e8080.L = readByte(addr + 1);
}

inline void PCHL (u8 opcode)
{
	e8080.PC = e8080.HL;
}

inline void JMP (u8 opcode)
{
	e8080.PC = fetch16();
}

inline void JC (u8 opcode)
{
	if (e8080.F & FLAG_CARRY) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void JNC (u8 opcode)
{
	if (!(e8080.F & FLAG_CARRY)) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void JZ (u8 opcode)
{
	if (e8080.F & FLAG_ZERO) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void JNZ (u8 opcode)
{
	if (!(e8080.F & FLAG_ZERO)) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void JM (u8 opcode)
{
	if (e8080.F & FLAG_SIGN) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void JP (u8 opcode)
{
	if (!(e8080.F & FLAG_SIGN)) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void JPE (u8 opcode)
{
	if (e8080.F & FLAG_PARITY) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void JPO (u8 opcode)
{
	if (!(e8080.F & FLAG_PARITY)) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void CALL (u8 opcode)
{
	u16 addr = fetch16();
	stackPush(e8080.PC);
	e8080.PC = addr;
}

inline void CC (u8 opcode)
{
	if (e8080.F & FLAG_CARRY) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void CNC (u8 opcode)
{
	if (!(e8080.F & FLAG_CARRY)) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void CZ (u8 opcode)
{
	if (e8080.F & FLAG_ZERO) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void CNZ (u8 opcode)
{
	if (!(e8080.F & FLAG_ZERO)) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void CM (u8 opcode)
{
	if (e8080.F & FLAG_SIGN) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void CP (u8 opcode)
{
	if (!(e8080.F & FLAG_SIGN)) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void CPE (u8 opcode)
{
	if (e8080.F & FLAG_PARITY) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void CPO (u8 opcode)
{
	if (!(e8080.F & FLAG_PARITY)) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

inline void RET (u8 opcode)
{
	e8080.PC = stackPop();
}

inline void RC (u8 opcode)
{
	if (e8080.F & FLAG_CARRY) {
		RET(opcode);
	}
}

inline void RNC (u8 opcode)
{
	if (!(e8080.F & FLAG_CARRY)) {
		RET(opcode);
	}
}

inline void RZ (u8 opcode)
{
	if (e8080.F & FLAG_ZERO) {
		RET(opcode);
	}
}

inline void RNZ (u8 opcode)
{
	if (!(e8080.F & FLAG_ZERO)) {
		RET(opcode);
	}
}

inline void RM (u8 opcode)
{
	if (e8080.F & FLAG_SIGN) {
		RET(opcode);
	}
}

inline void RP (u8 opcode)
{
	if (!(e8080.F & FLAG_SIGN)) {
		RET(opcode);
	}
}

inline void RPE (u8 opcode)
{
	if (e8080.F & FLAG_PARITY) {
		RET(opcode);
	}
}

inline void RPO (u8 opcode)
{
	if (!(e8080.F & FLAG_PARITY)) {
		RET(opcode);
	}
}

inline void RST (u8 opcode)
{
	stackPush(e8080.PC);
	e8080.PC = ((opcode >> 3) & 0x7) << 3;
}

inline void EI (u8 opcode)
{
	e8080.IE = 1;
}

inline void DI (u8 opcode)
{
	e8080.IE = 0;
}

inline void IN (u8 opcode)
{
	e8080.A = e8080.portIn(fetch8());
}

inline void OUT (u8 opcode)
{
	e8080.portOut(fetch8(), e8080.A);
}

inline void HLT (u8 opcode)
{
	e8080.halt = 1;
}

static struct 
{
	void (* execute) (u8 opcode);
	int cycles;
} opTbl [0X100] = {
	{  NOP, 4},
	{  LXI, 10},
	{  STAX, 7},
	{  INX, 5},
	{  INR, 5},
	{  DCR, 5},
	{  MVI, 7},
	{  RLC, 4},
	{  NOP, 4},
	{  DAD, 10},
	{  LDAX, 7}, 
	{  DCX, 5},
	{  INR, 5},
	{  DCR, 5},
	{  MVI, 7},
	{  RRC, 4}, // 0x10
	{  NOP, 4},
	{  LXI, 10},
	{  STAX, 7},
	{  INX, 5},
	{  INR, 5},
	{  DCR, 5},
	{  MVI, 7},
	{  RAL, 4},
	{  NOP, 4},
	{  DAD, 10},
	{  LDAX, 7},
	{  DCX, 5},
	{  INR, 5},
	{  DCR, 5},
	{ MVI, 7},
	{ RAR, 4},
	{ NOP, 4},// 0x20
	{ LXI, 10},
	{ SHLD, 16},
	{ INX, 5},
	{ INR, 5},
	{ DCR, 5},
	{ MVI, 7},
	{ DAA, 4},
	{ NOP, 4},
	{ DAD, 10},
	{ LHLD, 16},
	{ DCX, 5},
	{ INR, 5},
	{ DCR, 5},
	{ MVI, 7},
	{ CMA, 4},
	{ NOP, 4}, // 0x30
	{ LXI, 10},
	{ STA, 13},
	{ INX, 5},
	{ INR, 10},
	{ DCR, 10},
	{ MVI, 10},
	{ STC, 4},
	{ NOP, 4},
	{ DAD, 10},
	{ LDA, 13},
	{ DCX, 5},
	{ INR, 5},
	{ DCR, 5},
	{ MVI, 7},
	{ CMC, 4},
	{ MOV, 5}, // 0X40
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 7},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 7},
	{ MOV, 5},
	{ MOV, 5}, // 0X50
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 7},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 7},
	{ MOV, 5},
	{ MOV, 5}, // 0X60
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 7},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 7},
	{ MOV, 5},	
	{ MOV, 5}, // 0X70
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ HLT, 7},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 5},
	{ MOV, 7},
	{ MOV, 5},
	{ ADD, 4}, // 0X80
	{ ADD, 4},
	{ ADD, 4},
	{ ADD, 4},
	{ ADD, 4},
	{ ADD, 4},
	{ ADD, 7},
	{ ADD, 4},
	{ ADC, 4},
	{ ADC, 4},
	{ ADC, 4},
	{ ADC, 4},
	{ ADC, 4},
	{ ADC, 4},
	{ ADC, 7},
	{ ADC, 4},
	{ SUB, 4},
	{ SUB, 4},
	{ SUB, 4},
	{ SUB, 4},
	{ SUB, 4},
	{ SUB, 4},
	{ SUB, 7},
	{ SUB, 4},
	{ SBB, 4},
	{ SBB, 4},
	{ SBB, 4},
	{ SBB, 4},
	{ SBB, 4},
	{ SBB, 4},
	{ SBB, 7},
	{ SBB, 4},
	{ ANA, 4},
	{ ANA, 4},
	{ ANA, 4},
	{ ANA, 4},
	{ ANA, 4},
	{ ANA, 4},
	{ ANA, 7},
	{ ANA, 4},
	{ XRA, 4},
	{ XRA, 4},
	{ XRA, 4},
	{ XRA, 4},
	{ XRA, 4},
	{ XRA, 4},
	{ XRA, 7},
	{ XRA, 4},
	{ ORA, 4},
	{ ORA, 4},
	{ ORA, 4},
	{ ORA, 4},
	{ ORA, 4},
	{ ORA, 4},
	{ ORA, 7},
	{ ORA, 4},
	{ CMP, 4},
	{ CMP, 4},
	{ CMP, 4},
	{ CMP, 4},
	{ CMP, 4},
	{ CMP, 4},
	{ CMP, 7},
	{ CMP, 4},
	{ RNZ, 11},
	{ POP, 10},
	{ JNZ, 10},
	{ JMP, 10},
	{ CNZ, 17},
	{ PUSH, 11},
	{ ADI, 7},
	{ RST, 11},
	{ RZ, 11},
	{ RET, 10},
	{ JZ, 10},
	{ JMP, 10},
	{ CZ, 17},
	{ CALL, 17},
	{ ACI, 7},
	{ RST, 11},
	{ RNC, 11},
	{ POP, 10},
	{ JNC, 10},
	{ OUT, 10},
	{ CNC, 17},
	{ PUSH, 11},
	{ SUI, 7},
	{ RST, 11},
	{ RC, 11},
	{ RET, 10},
	{ JC, 10},
	{ IN, 10},
	{ CC, 17},
	{ CALL, 17},
	{ SBI, 7},
	{ RST, 11},
	{ RPO, 11},
	{ POP, 10},
	{ JPO, 10},
	{ XTHL, 18},
	{ CPO, 17},
	{ PUSH, 11},
	{ ANI, 7},
	{ RST, 11},
	{ RPE, 11},
	{ PCHL, 5},
	{ JPE, 10},
	{ XCHG, 5},
	{ CPE, 17},
	{ CALL, 17},
	{ XRI, 7},
	{ RST, 11},
	{ RP, 11},
	{ POP, 10},
	{ JP, 10},
	{ DI, 4},
	{ CP, 17},
	{ PUSH, 11},
	{ ORI, 7},
	{ RST, 11},
	{ RM, 11},
	{ SPHL, 5},
	{ JM, 10},
	{ EI, 4},
	{ CM, 17},
	{ CALL, 17},
	{ CPI, 7},
	{ RST, 11}
};

#endif


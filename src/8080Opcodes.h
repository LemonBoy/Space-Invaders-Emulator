#ifndef E8080OPCODES_H
#define E8080OPCODES_H

uint8_t fetch8 ()
{
	if (e8080.PC >= 0x9000) {
		assert(0);
		dumpRegisters();
	}
	return readByte(e8080.PC++);
}

uint16_t fetch16 ()
{
	uint16_t r = fetch8() | fetch8() << 8 ;
	return r;
}

void setFlags (uint32_t reg, int fMask)
{
	e8080.F &= ~((1 << 5) | (1 << 3) | fMask);
	e8080.F |= 0x02;
	reg &= 0xFFFF;

	if (fMask & FLAG_SIGN)
	{
		if (reg & (1 << 7)) {
			e8080.F |= FLAG_SIGN;
		}
	}

	if (fMask & FLAG_ZERO)
	{
		if (!(reg&0xff)) {
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
		uint8_t rr = reg&0xff;
		int pf = 0;
		while (rr)
		{
			pf ^= (rr&1);
			rr >>= 1;
		}
		if (!(pf&1)) {
			e8080.F |= FLAG_PARITY;
		}
	}
}

void writeReg (uint8_t value, int index)
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
		default:
			assert(0);
	}
}

uint8_t readReg (int index)
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
		default:
			assert(0);
	}
	return 0xFF;
}

void CMC (uint8_t opcode)
{
	e8080.F ^= FLAG_CARRY;
}

void STC (uint8_t opcode)
{
	e8080.F |= FLAG_CARRY;
}

void DAA (uint8_t opcode)
{
	if (((e8080.A&0xf) > 9) || (e8080.F & FLAG_ACARRY)) {
		if ((e8080.A&0xF) + 6 > 15)
			e8080.F |= FLAG_ACARRY;
		else
			e8080.F &= ~FLAG_ACARRY;
		setFlags(e8080.A + 6, FLAG_CARRY);
		e8080.A += 6;
	}

	if (((e8080.A>>4) > 9) || (e8080.F & FLAG_CARRY)) {
		if ((e8080.A + (6 << 4)) & 0x100)
			e8080.F |= FLAG_CARRY;
		e8080.A += (6 << 4);
	}

	setFlags(e8080.A, FLAG_ZERO | FLAG_SIGN | FLAG_PARITY);
}

void INR (uint8_t opcode)
{
	int dst = (opcode >> 3) & 0x7;
	uint16_t tmp = readReg(dst) + 1;
	setFlags(tmp, FLAG_SIGN | FLAG_ZERO | FLAG_PARITY);
	if ((readReg(dst) & 0xf) > (tmp & 0xf)) {
		e8080.F |= FLAG_ACARRY;
	}
	else {
		e8080.F &= ~FLAG_ACARRY;
	}
	writeReg(tmp, dst);
}

void DCR (uint8_t opcode)
{
	int dst = (opcode >> 3) & 0x7;
	uint16_t tmp = readReg(dst) - 1;
	setFlags(tmp, FLAG_SIGN | FLAG_ZERO | FLAG_PARITY);
	if ((readReg(dst) & 0xf) > (tmp & 0xf)) {
		e8080.F |= FLAG_ACARRY;
	}
	else {
		e8080.F &= ~FLAG_ACARRY;
	}
	writeReg(tmp, dst);
}

void CMA (uint8_t opcode)
{
	e8080.A = 0xff ^ e8080.A;
}

void NOP (uint8_t opcode)
{
}

void MOV (uint8_t opcode)
{
	int dst = (opcode >> 3) & 0x7;
	int src = (opcode & 0x7);
	uint8_t srcVal = readReg(src);
	writeReg(srcVal, dst);
}

void STAX (uint8_t opcode)
{
	int src = (opcode >> 4) & 1;
	uint16_t addr = (!src) ? e8080.BC : e8080.DE;
	writeByte(e8080.A, addr);
}

void LDAX (uint8_t opcode)
{
	int src = (opcode >> 4) & 1;
	uint16_t addr = (!src) ? e8080.BC : e8080.DE;
	e8080.A = readByte(addr);
}

void ADD (uint8_t opcode)
{
	int src = (opcode & 0x7);
	uint8_t srcVal = readReg(src);
	setFlags(e8080.A + srcVal, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A += srcVal;
}

void ADC (uint8_t opcode)
{
	int src = (opcode & 0x7);
	uint8_t srcVal = readReg(src);
	uint16_t tmp = e8080.A + srcVal + !!(e8080.F & FLAG_CARRY);
	setFlags(tmp, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A = tmp&0xff;
}

void SUB (uint8_t opcode)
{
	int src = (opcode & 0x7);
	uint8_t srcVal = readReg(src);
	setFlags(e8080.A - srcVal, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A -= srcVal;
}

void SBB (uint8_t opcode)
{
	int src = (opcode & 0x7);
	uint8_t srcVal = readReg(src);
	uint16_t tmp = e8080.A - (srcVal + !!(e8080.F & FLAG_CARRY));
	setFlags(tmp, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A = tmp&0xff;
}

void ANA (uint8_t opcode)
{
	int src = (opcode & 0x7);
	uint8_t srcVal = readReg(src);
	setFlags(e8080.A & srcVal, FLAG_SIGN | FLAG_ZERO | FLAG_PARITY);
	e8080.F &= ~FLAG_CARRY;
	e8080.F &= ~FLAG_ACARRY;
	if ((e8080.A | srcVal) & 0x08) {
		e8080.F |= FLAG_ACARRY;
	}
	e8080.A &= srcVal;
}

void XRA (uint8_t opcode)
{
	int src = (opcode & 0x7);
	uint8_t srcVal = readReg(src);
	setFlags(e8080.A ^ srcVal, FLAG_SIGN | FLAG_ZERO | FLAG_PARITY);
	e8080.F &= ~FLAG_CARRY;
	e8080.A ^= srcVal;
	e8080.F &= ~FLAG_ACARRY;
}

void ORA (uint8_t opcode)
{
	int src = (opcode & 0x7);
	uint8_t srcVal = readReg(src);
	setFlags(e8080.A | srcVal, FLAG_SIGN | FLAG_ZERO | FLAG_PARITY);
	e8080.F &= ~FLAG_CARRY;
	e8080.A |= srcVal;
	e8080.F &= ~FLAG_ACARRY;
}

void CMP (uint8_t opcode)
{
	int src = (opcode & 0x7);
	uint8_t srcVal = readReg(src);
	setFlags(e8080.A - srcVal, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
}

void RLC (uint8_t opcode)
{
	e8080.F &= ~(FLAG_CARRY);
	if (e8080.A & (1 << 7)) {
		e8080.F |= FLAG_CARRY;
	}
	e8080.A = (e8080.A << 1) | !!(e8080.F & FLAG_CARRY);
}

void RRC (uint8_t opcode)
{
	e8080.F &= ~(FLAG_CARRY);
	if (e8080.A & 1) {
		e8080.F |= FLAG_CARRY;
	}
	e8080.A = (e8080.A >> 1) | (!!(e8080.F & FLAG_CARRY) << 7);
}

void RAL (uint8_t opcode)
{
	uint8_t c = (e8080.A & (1 << 7));
	e8080.A = (e8080.A << 1) | !!(e8080.F & FLAG_CARRY);
	if (c) e8080.F |= FLAG_CARRY;
	else   e8080.F &= ~FLAG_CARRY;
}

void RAR (uint8_t opcode)
{
	uint8_t c = (e8080.A & 1);
	e8080.A = (e8080.A >> 1) | (!!(e8080.F & FLAG_CARRY) << 7);
	if (c) e8080.F |= FLAG_CARRY;
	else   e8080.F &= ~FLAG_CARRY;
}

void PUSH (uint8_t opcode)
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
			assert(0);
	}
}

void POP (uint8_t opcode)
{
	int src = (opcode >> 4) & 3;
	uint16_t val = stackPop();
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
			e8080.AF = (val & 0xFFD7) | 0x02;
			break;
		default:
			assert(0);
	}
}

void DAD (uint8_t opcode)
{
	int src = (opcode >> 4) & 3;
	uint16_t rVal = 0;
	switch (src) {
		case 0x00:
			rVal = e8080.BC; break;
		case 0x01:
			rVal = e8080.DE; break;
		case 0x02:
			rVal = e8080.HL; break;
		case 0x03:
			rVal = e8080.SP; break;
		default:
			assert(0);
	}
	e8080.F &= ~FLAG_CARRY;
	if ((e8080.HL + rVal) & 0x10000)
		e8080.F |= FLAG_CARRY;
	e8080.HL += rVal;
}

void INX (uint8_t opcode)
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
			assert(0);
	}
}

void DCX (uint8_t opcode)
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
			assert(0);
	}
}

void XCHG (uint8_t opcode)
{
	uint16_t tmp = e8080.DE;
	e8080.DE = e8080.HL;
	e8080.HL = tmp;
}

void XTHL (uint8_t opcode)
{
	uint8_t old_h = e8080.H;
	uint8_t old_l = e8080.L;

	e8080.L = readByte(e8080.SP + 0);
	writeByte(old_l, e8080.SP + 0);
	e8080.H = readByte(e8080.SP + 1);
	writeByte(old_h, e8080.SP + 1);
}

void SPHL (uint8_t opcode)
{
	e8080.SP = e8080.HL;
}

void LXI (uint8_t opcode)
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
			assert(0);
	}
}

void MVI (uint8_t opcode)
{
	int dst = (opcode >> 3) & 0x7;
	writeReg(fetch8(), dst);
}

void ADI (uint8_t opcode)
{
	uint8_t imm = fetch8();
	setFlags(e8080.A + imm, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A += imm;
}

void ACI (uint8_t opcode)
{
	uint8_t imm = fetch8();
	uint16_t tmp = e8080.A + imm + !!(e8080.F & FLAG_CARRY);
	setFlags(tmp, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	e8080.A = tmp;
}

void SUI (uint8_t opcode)
{
	uint8_t imm = fetch8();
	setFlags(e8080.A - imm, FLAG_SIGN | FLAG_ZERO | FLAG_PARITY);
	if ((e8080.A - imm) > 0xFF)
		e8080.F &= ~FLAG_CARRY;
	else
		e8080.F |= FLAG_CARRY;
	if (((e8080.A - imm) & 0xF) > (e8080.A&0xF))
		e8080.F &= ~FLAG_ACARRY;
	else
		e8080.F |= FLAG_ACARRY;
	e8080.A -= imm;
}

void SBI (uint8_t opcode)
{
	uint8_t imm = fetch8();
	uint16_t tmp = e8080.A - imm - !!(e8080.F & FLAG_CARRY);
	setFlags(tmp, FLAG_SIGN | FLAG_ZERO | FLAG_PARITY);
	if ((e8080.A - imm) > 0xFF)
		e8080.F &= ~FLAG_CARRY;
	else
		e8080.F |= FLAG_CARRY;
	if (((e8080.A - imm) & 0xF) > (e8080.A&0xF))
		e8080.F &= ~FLAG_ACARRY;
	else
		e8080.F |= FLAG_ACARRY;
	e8080.A = tmp;
}

void ANI (uint8_t opcode)
{
	uint8_t imm = fetch8();
	setFlags(e8080.A & imm, FLAG_SIGN | FLAG_ZERO | FLAG_PARITY);
	e8080.F &= ~FLAG_CARRY;
	e8080.F &= ~FLAG_ACARRY;
	if ((e8080.A | imm) & 0x08) {
		e8080.F |= FLAG_ACARRY;
	}
	e8080.A &= imm;
}

void XRI (uint8_t opcode)
{
	uint8_t imm = fetch8();
	setFlags(e8080.A ^ imm, FLAG_SIGN | FLAG_ZERO | FLAG_PARITY);
	e8080.F &= ~FLAG_CARRY;
	e8080.F &= ~FLAG_ACARRY;
	e8080.A ^= imm;
}

void ORI (uint8_t opcode)
{
	uint8_t imm = fetch8();
	setFlags(e8080.A | imm, FLAG_SIGN | FLAG_ZERO | FLAG_PARITY);
	e8080.F &= ~FLAG_CARRY;
	e8080.F &= ~FLAG_ACARRY;
	e8080.A |= imm;
}

void CPI (uint8_t opcode)
{
	uint8_t imm = fetch8();
	// setFlags(e8080.A - imm, FLAG_SIGN | FLAG_ZERO | FLAG_ACARRY | FLAG_PARITY | FLAG_CARRY);
	setFlags(e8080.A - imm, FLAG_SIGN | FLAG_ZERO | FLAG_PARITY);
	setFlags(e8080.A + ~imm + 1, FLAG_CARRY | FLAG_ACARRY);
	// if ((e8080.A - imm) > 0xFF)
	// 	e8080.F &= ~FLAG_CARRY;
	// else
	// 	e8080.F |= FLAG_CARRY;
	// if (((e8080.A - imm) & 0xF) > (e8080.A&0xF))
	// 	e8080.F &= ~FLAG_ACARRY;
	// else
	// 	e8080.F |= FLAG_ACARRY;
}

void STA (uint8_t opcode)
{
	uint16_t addr = fetch16();
	writeByte(e8080.A, addr);
}

void LDA (uint8_t opcode)
{
	uint16_t addr = fetch16();
	e8080.A = readByte(addr);
}

void SHLD (uint8_t opcode)
{
	uint16_t addr = fetch16();
	writeByte(e8080.H, addr + 1);
	writeByte(e8080.L, addr + 0);
}

void LHLD (uint8_t opcode)
{
	uint16_t addr = fetch16();
	e8080.H = readByte(addr + 1);
	e8080.L = readByte(addr + 0);
}

void PCHL (uint8_t opcode)
{
	e8080.PC = e8080.HL;
}

void JMP (uint8_t opcode)
{
	e8080.PC = fetch16();
}

void JC (uint8_t opcode)
{
	if (e8080.F & FLAG_CARRY) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

void JNC (uint8_t opcode)
{
	if (!(e8080.F & FLAG_CARRY)) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

void JZ (uint8_t opcode)
{
	if (e8080.F & FLAG_ZERO) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

void JNZ (uint8_t opcode)
{
	if (!(e8080.F & FLAG_ZERO)) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

void JM (uint8_t opcode)
{
	if (e8080.F & FLAG_SIGN) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

void JP (uint8_t opcode)
{
	if (!(e8080.F & FLAG_SIGN)) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

void JPE (uint8_t opcode)
{
	if (e8080.F & FLAG_PARITY) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

void JPO (uint8_t opcode)
{
	if (!(e8080.F & FLAG_PARITY)) {
		JMP(opcode);
	} else {
		e8080.PC += 2;
	}
}

void CALL (uint8_t opcode)
{
	uint16_t addr = fetch16();
	if (addr == 0x5)
	{
		if (e8080.C == 0x09) {
			int i = 0;
			while (1) {
				uint8_t ch = readByte(e8080.DE + i);
				if (ch == '$') {
					break;
				}
				fputc(ch, stderr);
				i++;
			}
		}
		else if (e8080.C == 0x02) {
			uint8_t ch = e8080.E;
			fputc(ch, stderr);
		}
		else {
			// Unhandled FDOS instruction
			fprintf(stderr, "unhandled FDOS fn=%d\n", e8080.C);
			assert(0);
		}
	}
	else {
		stackPush(e8080.PC);
		e8080.PC = addr;
	}
}

void CC (uint8_t opcode)
{
	if (e8080.F & FLAG_CARRY) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

void CNC (uint8_t opcode)
{
	if (!(e8080.F & FLAG_CARRY)) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

void CZ (uint8_t opcode)
{
	if (e8080.F & FLAG_ZERO) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

void CNZ (uint8_t opcode)
{
	if (!(e8080.F & FLAG_ZERO)) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

void CM (uint8_t opcode)
{
	if (e8080.F & FLAG_SIGN) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

void CP (uint8_t opcode)
{
	if (!(e8080.F & FLAG_SIGN)) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

void CPE (uint8_t opcode)
{
	if (e8080.F & FLAG_PARITY) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

void CPO (uint8_t opcode)
{
	if (!(e8080.F & FLAG_PARITY)) {
		CALL(opcode);
	} else {
		e8080.PC += 2;
	}
}

void RET (uint8_t opcode)
{
	e8080.PC = stackPop();
}

void RC (uint8_t opcode)
{
	if (e8080.F & FLAG_CARRY) {
		RET(opcode);
	}
}

void RNC (uint8_t opcode)
{
	if (!(e8080.F & FLAG_CARRY)) {
		RET(opcode);
	}
}

void RZ (uint8_t opcode)
{
	if (e8080.F & FLAG_ZERO) {
		RET(opcode);
	}
}

void RNZ (uint8_t opcode)
{
	if (!(e8080.F & FLAG_ZERO)) {
		RET(opcode);
	}
}

void RM (uint8_t opcode)
{
	if (e8080.F & FLAG_SIGN) {
		RET(opcode);
	}
}

void RP (uint8_t opcode)
{
	if (!(e8080.F & FLAG_SIGN)) {
		RET(opcode);
	}
}

void RPE (uint8_t opcode)
{
	if (e8080.F & FLAG_PARITY) {
		RET(opcode);
	}
}

void RPO (uint8_t opcode)
{
	if (!(e8080.F & FLAG_PARITY)) {
		RET(opcode);
	}
}

void RST (uint8_t opcode)
{
	stackPush(e8080.PC);
	e8080.PC = ((opcode >> 3) & 0x7) << 3;
}

void EI (uint8_t opcode)
{
	e8080.IE = 1;
}

void DI (uint8_t opcode)
{
	e8080.IE = 0;
}

void IN (uint8_t opcode)
{
	e8080.A = e8080.portIn(fetch8());
}

void OUT (uint8_t opcode)
{
	e8080.portOut(fetch8(), e8080.A);
}

void HLT (uint8_t opcode)
{
	e8080.halt = 1;
}

static struct
{
	void (* execute) (uint8_t opcode);
	int cycles;
} opTbl [0x100] = {
	{ NOP, 4},
	{ LXI, 10},
	{ STAX, 7},
	{ INX, 5},
	{ INR, 5},
	{ DCR, 5},
	{ MVI, 7},
	{ RLC, 4},
	{ NOP, 4},
	{ DAD, 10},
	{ LDAX, 7},
	{ DCX, 5},
	{ INR, 5},
	{ DCR, 5},
	{ MVI, 7},
	{ RRC, 4},
	{ NOP, 4}, // 0x10
	{ LXI, 10},
	{ STAX, 7},
	{ INX, 5},
	{ INR, 5},
	{ DCR, 5},
	{ MVI, 7},
	{ RAL, 4},
	{ NOP, 4},
	{ DAD, 10},
	{ LDAX, 7},
	{ DCX, 5},
	{ INR, 5},
	{ DCR, 5},
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


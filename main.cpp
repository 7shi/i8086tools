#include "disasm.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

uint16_t ip, r[8];
uint8_t *r8[8];
uint8_t text[65536], mem[65536], *data;
bool OF, SF, ZF, PF, CF;
bool ptable[256];

#define AX r[0]
#define CX r[1]
#define DX r[2]
#define BX r[3]
#define SP r[4]
#define BP r[5]
#define SI r[6]
#define DI r[7]
#define AL *r8[0]
#define CL *r8[1]
#define DL *r8[2]
#define BL *r8[3]
#define AH *r8[4]
#define CH *r8[5]
#define DH *r8[6]
#define BH *r8[7]

const char *header = " AX   CX   DX   BX   SP   BP   SI   DI  FLAGS  IP\n";

static void debug() {
	fprintf(stderr,
		"%04x %04x %04x %04x %04x %04x %04x %04x %c%c%c%c%c %04x",
		r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7],
		OF?'O':'-', SF?'S':'-', ZF?'Z':'-', PF?'P':'-', CF?'C':'-', ip);
}

static void init_table() {
	uint16_t tmp = 0x1234;
	uint8_t *p = (uint8_t *)r;
	if (*(uint8_t *)&tmp == 0x34) {
		for (int i = 0; i < 4; i++) {
			r8[i] = p + i * 2;
			r8[i + 4] = r8[i] + 1;
		}
	} else {
		for (int i = 0; i < 4; i++) {
			r8[i] = p + i * 2 + 1;
			r8[i + 4] = r8[i] - 1;
		}
	}
	for (int i = 0; i < 256; i++) {
		int n = 0;
		for (int j = 1; j < 256; j += j) {
			if (i & j) n++;
		}
		ptable[i] = (n & 1) == 0;
	}
}

inline uint16_t read16(uint16_t addr) {
	return data[addr] | (data[addr + 1] << 8);
}

inline void write16(uint16_t addr, uint16_t value) {
	data[addr] = value;
	data[addr + 1] = value >> 8;
}

static int addr(const Operand &opr) {
	switch (opr.type) {
	case Ptr: return uint16_t(opr.value);
	case ModRM + 0: return uint16_t(BX + SI + opr.value);
	case ModRM + 1: return uint16_t(BX + DI + opr.value);
	case ModRM + 2: return uint16_t(BP + SI + opr.value);
	case ModRM + 3: return uint16_t(BP + DI + opr.value);
	case ModRM + 4: return uint16_t(     SI + opr.value);
	case ModRM + 5: return uint16_t(     DI + opr.value);
	case ModRM + 6: return uint16_t(BP      + opr.value);
	case ModRM + 7: return uint16_t(BX      + opr.value);
	}
	return -1;
}

static uint8_t get8(const Operand &opr) {
	switch (opr.type) {
	case Reg: return *r8[opr.value];
	case Imm: return opr.value;
	}
	int ad = addr(opr);
	return ad < 0 ? 0 : data[ad];
}

static uint16_t get16(const Operand &opr) {
	switch (opr.type) {
	case Reg: return r[opr.value];
	case Imm: return opr.value;
	}
	int ad = addr(opr);
	return ad < 0 ? 0 : read16(ad);
}

static void set8(const Operand &opr, uint8_t value) {
	if (opr.type == Reg) {
		*r8[opr.value] = value;
	} else {
		int ad = addr(opr);
		if (ad >= 0) data[ad] = value;
	}
}

static void set16(const Operand &opr, uint16_t value) {
	if (opr.type == Reg) {
		r[opr.value] = value;
	} else {
		int ad = addr(opr);
		if (ad >= 0) write16(ad, value);
	}
}

inline int setf8(int value, bool cf) {
	int8_t v = value;
	OF = value != v;
	SF = v < 0;
	ZF = v == 0;
	PF = ptable[uint8_t(value)];
	CF = cf;
	return value;
}

inline int setf16(int value, bool cf) {
	int16_t v = value;
	OF = value != v;
	SF = v < 0;
	ZF = v == 0;
	PF = ptable[uint8_t(value)];
	CF = cf;
	return value;
}

static bool run1() {
	OpCode op = disasm1(text + ip, ip);
	int opr1 = op.opr1.value, opr2 = op.opr2.value;
	std::string hex = hexdump(text + ip, op.len);
	debug();
	fprintf(stderr, ":%-12s %s\n", hex.c_str(), op.str().c_str());
	uint8_t b = text[ip];
	uint16_t oldip = ip;
	int dst, src, val;
	ip += op.len;
	switch (b) {
	case 0x00: // add r/m, reg8
		val = int8_t(dst = get8(op.opr1)) + int8_t(*r8[opr2]);
		set8(op.opr1, setf8(val, dst > uint8_t(val)));
		return true;
	case 0x01: // add r/m, reg16
		val = int16_t(dst = get16(op.opr1)) + int16_t(r[opr2]);
		set16(op.opr1, setf16(val, dst > uint16_t(val)));
		return true;
	case 0x02: // add reg8, r/m
		val = int8_t(dst = *r8[opr1]) + int8_t(get8(op.opr2));
		*r8[opr1] = setf8(val, dst > uint8_t(val));
		return true;
	case 0x03: // add reg16, r/m
		val = int16_t(dst = r[opr1]) + int16_t(get16(op.opr2));
		r[opr1] = setf16(val, dst > uint16_t(val));
		return true;
	case 0x04: // add al, imm8
		val = int8_t(dst = AL) + int8_t(opr2);
		AL = setf8(val, dst > uint8_t(val));
		return true;
	case 0x05: // add ax, imm16
		val = int16_t(dst = AX) + int16_t(opr2);
		AX = setf16(val, dst > uint16_t(val));
		return true;
	case 0x08: // or r/m, reg8
		set8(op.opr1, setf8(int8_t(get8(op.opr1) | *r8[opr2]), false));
		return true;
	case 0x09: // or r/m, reg16
		set16(op.opr1, setf16(int16_t(get16(op.opr1) | r[opr2]), false));
		return true;
	case 0x0a: // or reg8, r/m
		*r8[opr1] = setf8(int8_t(*r8[opr1] | get8(op.opr2)), false);
		return true;
	case 0x0b: // or reg16, r/m
		r[opr1] = setf16(int16_t(r[opr1] | get16(op.opr2)), false);
		return true;
	case 0x0c: // or al, imm8
		AL = setf8(int8_t(AL | opr2), false);
		return true;
	case 0x0d: // or ax, imm16
		AX = setf16(int16_t(AX | opr2), false);
		return true;
	case 0x10: // adc r/m, reg8
		val = int8_t(dst = get8(op.opr1)) + int8_t(*r8[opr2]) + int(CF);
		set8(op.opr1, setf8(val, dst > uint8_t(val)));
		return true;
	case 0x11: // adc r/m, reg16
		val = int16_t(dst = get16(op.opr1)) + int16_t(r[opr2]) + int(CF);
		set16(op.opr1, setf16(val, dst > uint16_t(val)));
		return true;
	case 0x12: // adc reg8, r/m
		val = int8_t(dst = *r8[opr1]) + int8_t(get8(op.opr2)) + int(CF);
		*r8[opr1] = setf8(val, dst > uint8_t(val));
		return true;
	case 0x13: // adc reg16, r/m
		val = int16_t(dst = r[opr1]) + int16_t(get16(op.opr2)) + int(CF);
		r[opr1] = setf16(val, dst > uint16_t(val));
		return true;
	case 0x14: // adc al, imm8
		val = int8_t(dst = AL) + int8_t(opr2) + int(CF);
		AL = setf8(val, dst > uint8_t(val));
		return true;
	case 0x15: // adc ax, imm16
		val = int16_t(dst = AX) + int16_t(opr2) + int(CF);
		AX = setf16(val, dst > uint16_t(val));
		return true;
	case 0x18: // sbb r/m, reg8
		val = int8_t(dst = get8(op.opr1)) - int8_t(src = *r8[opr2] + int(CF));
		set8(op.opr1, setf8(val, dst < src));
		return true;
	case 0x19: // sbb r/m, reg16
		val = int16_t(dst = get16(op.opr1)) - int16_t(src = r[opr2] + int(CF));
		set16(op.opr1, setf16(val, dst < src));
		return true;
	case 0x1a: // sbb reg8, r/m
		val = int8_t(dst = *r8[opr1]) - int8_t(src = get8(op.opr2) + int(CF));
		*r8[opr1] = setf8(val, dst < src);
		return true;
	case 0x1b: // sbb reg16, r/m
		val = int16_t(dst = r[opr1]) - int16_t(src = get16(op.opr2) + int(CF));
		r[opr1] = setf16(val, dst < src);
		return true;
	case 0x1c: // sbb al, imm8
		val = int8_t(dst = AL) - int8_t(src = opr2 + int(CF));
		AL = setf8(val, dst < src);
		return true;
	case 0x1d: // sbb ax, imm16
		val = int16_t(dst = AX) - int16_t(src = opr2 + int(CF));
		AX = setf16(val, dst < src);
		return true;
	case 0x20: // and r/m, reg8
		set8(op.opr1, setf8(int8_t(get8(op.opr1) & *r8[opr2]), false));
		return true;
	case 0x21: // and r/m, reg16
		set16(op.opr1, setf16(int16_t(get16(op.opr1) & r[opr2]), false));
		return true;
	case 0x22: // and reg8, r/m
		*r8[opr1] = setf8(int8_t(*r8[opr1] & get8(op.opr2)), false);
		return true;
	case 0x23: // and reg16, r/m
		r[opr1] = setf16(int16_t(r[opr1] & get16(op.opr2)), false);
		return true;
	case 0x24: // and al, imm8
		AL = setf8(int8_t(AL & opr2), false);
		return true;
	case 0x25: // and ax, imm16
		AX = setf16(int16_t(AX & opr2), false);
		return true;
	case 0x28: // sub r/m, reg8
		val = int8_t(dst = get8(op.opr1)) - int8_t(src = *r8[opr2]);
		set8(op.opr1, setf8(val, dst < src));
		return true;
	case 0x29: // sub r/m, reg16
		val = int16_t(dst = get16(op.opr1)) - int16_t(src = r[opr2]);
		set16(op.opr1, setf16(val, dst < src));
		return true;
	case 0x2a: // sub reg8, r/m
		val = int8_t(dst = *r8[opr1]) - int8_t(src = get8(op.opr2));
		*r8[opr1] = setf8(val, dst < src);
		return true;
	case 0x2b: // sub reg16, r/m
		val = int16_t(dst = r[opr1]) - int16_t(src = get16(op.opr2));
		r[opr1] = setf16(val, dst < src);
		return true;
	case 0x2c: // sub al, imm8
		val = int8_t(dst = AL) - int8_t(src = opr2);
		AL = setf8(val, dst < src);
		return true;
	case 0x2d: // sub ax, imm16
		val = int16_t(dst = AX) - int16_t(src = opr2);
		AX = setf16(val, dst < src);
		return true;
	case 0x30: // xor r/m, reg8
		set8(op.opr1, setf8(int8_t(get8(op.opr1) ^ *r8[opr2]), false));
		return true;
	case 0x31: // xor r/m, reg16
		set16(op.opr1, setf16(int16_t(get16(op.opr1) ^ r[opr2]), false));
		return true;
	case 0x32: // xor reg8, r/m
		*r8[opr1] = setf8(int8_t(*r8[opr1] ^ get8(op.opr2)), false);
		return true;
	case 0x33: // xor reg16, r/m
		r[opr1] = setf16(int16_t(r[opr1] ^ get16(op.opr2)), false);
		return true;
	case 0x34: // xor al, imm8
		AL = setf8(int8_t(AL ^ opr2), false);
		return true;
	case 0x35: // xor ax, imm16
		AX = setf16(int16_t(AX ^ opr2), false);
		return true;
	case 0x38: // cmp r/m, reg8
		val = int8_t(dst = get8(op.opr1)) - int8_t(src = *r8[opr2]);
		setf8(val, dst < src);
		return true;
	case 0x39: // cmp r/m, reg16
		val = int16_t(dst = get16(op.opr1)) - int16_t(src = r[opr2]);
		setf16(val, dst < src);
		return true;
	case 0x3a: // cmp reg8, r/m
		val = int8_t(dst = *r8[opr1]) - int8_t(src = get8(op.opr2));
		setf8(val, dst < src);
		return true;
	case 0x3b: // cmp reg16, r/m
		val = int16_t(dst = r[opr1]) - int16_t(src = get16(op.opr2));
		setf16(val, dst < src);
		return true;
	case 0x3c: // cmp al, imm8
		val = int8_t(dst = AL) - int8_t(src = opr2);
		setf8(val, dst < src);
		return true;
	case 0x3d: // cmp ax, imm16
		val = int16_t(dst = AX) - int16_t(src = opr2);
		setf16(val, dst < src);
		return true;
	case 0x50: // push reg16
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
		SP -= 2;
		write16(SP, r[opr1]);
		return true;
	case 0x58: // pop reg16
	case 0x59:
	case 0x5a:
	case 0x5b:
	case 0x5c:
	case 0x5d:
	case 0x5e:
	case 0x5f:
		r[opr1] = read16(SP);
		SP += 2;
		return true;
	case 0x70: // jo
		if (OF) ip = opr1;
		return true;
	case 0x71: // jno
		if (!OF) ip = opr1;
		return true;
	case 0x72: // jb/jnae
		if (CF) ip = opr1;
		return true;
	case 0x73: // jnb/jae
		if (!CF) ip = opr1;
		return true;
	case 0x74: // je/jz
		if (ZF) ip = opr1;
		return true;
	case 0x75: // jne/jnz
		if (!ZF) ip = opr1;
		return true;
	case 0x76: // jbe/jna
		if (CF || ZF) ip = opr1;
		return true;
	case 0x77: // jnbe/ja
		if (!CF && !ZF) ip = opr1;
		return true;
	case 0x78: // js
		if (SF) ip = opr1;
		return true;
	case 0x79: // jns
		if (!SF) ip = opr1;
		return true;
	case 0x7a: // jp
		if (PF) ip = opr1;
		return true;
	case 0x7b: // jnp
		if (!PF) ip = opr1;
		return true;
	case 0x7c: // jl/jnge
		if (SF != OF) ip = opr1;
		return true;
	case 0x7d: // jnl/jge
		if (SF == OF) ip = opr1;
		return true;
	case 0x7e: // jle/jng
		if (ZF || SF != OF) ip = opr1;
		return true;
	case 0x7f: // jnle/jg
		if (!ZF && SF == OF) ip = opr1;
		return true;
	case 0x80: // r/m, imm8
		switch ((text[oldip + 1] >> 3) & 7) {
		case 0: // add
			val = int8_t(dst = get8(op.opr1)) + int8_t(opr2);
			set8(op.opr1, setf8(val, dst > uint8_t(val)));
			return true;
		case 1: // or
			set8(op.opr1, setf8(int8_t(get8(op.opr1) | opr2), false));
			return true;
		case 2: // adc
			val = int8_t(dst = get8(op.opr1)) + int8_t(opr2) + int(CF);
			set8(op.opr1, setf8(val, dst > uint8_t(val)));
			return true;
		case 3: // sbb
			val = int8_t(dst = get8(op.opr1)) - int8_t(src = opr2 + int(CF));
			set8(op.opr1, setf8(val, dst < src));
			return true;
		case 4: // and
			set8(op.opr1, setf8(int8_t(get8(op.opr1) & opr2), false));
			return true;
		case 5: // sub
			val = int8_t(dst = get8(op.opr1)) - int8_t(src = opr2);
			set8(op.opr1, setf8(val, dst < src));
			return true;
		case 6: // xor
			set8(op.opr1, setf8(int8_t(get8(op.opr1) ^ opr2), false));
			return true;
		case 7: // cmp
			val = int8_t(dst = get8(op.opr1)) - int8_t(src = opr2);
			setf8(val, dst < src);
			return true;
		}
		break;
	case 0x81: // r/m, imm16
		switch ((text[oldip + 1] >> 3) & 7) {
		case 0: // add
			val = int16_t(dst = get16(op.opr1)) + int16_t(opr2);
			set16(op.opr1, setf16(val, dst > uint16_t(val)));
			return true;
		case 1: // or
			set16(op.opr1, setf16(int16_t(get16(op.opr1) | opr2), false));
			return true;
		case 2: // adc
			val = int16_t(dst = get16(op.opr1)) + int16_t(opr2) + int(CF);
			set16(op.opr1, setf16(val, dst > uint16_t(val)));
			return true;
		case 3: // sbb
			val = int16_t(dst = get16(op.opr1)) - int16_t(src = opr2 + int(CF));
			set16(op.opr1, setf16(val, dst < src));
			return true;
		case 4: // and
			set16(op.opr1, setf16(int16_t(get16(op.opr1) & opr2), false));
			return true;
		case 5: // sub
			val = int16_t(dst = get16(op.opr1)) - int16_t(src = opr2);
			set16(op.opr1, setf16(val, dst < src));
			return true;
		case 6: // xor
			set16(op.opr1, setf16(int16_t(get16(op.opr1) ^ opr2), false));
			return true;
		case 7: // cmp
			val = int16_t(dst = get16(op.opr1)) - int16_t(src = opr2);
			setf16(val, dst < src);
			return true;
		}
		break;
	case 0x82: // r/m, imm8(signed)
		switch ((text[oldip + 1] >> 3) & 7) {
		case 0: // add
			val = int8_t(dst = get8(op.opr1)) + int8_t(opr2);
			set8(op.opr1, setf8(val, dst > uint8_t(val)));
			return true;
		case 2: // adc
			val = int8_t(dst = get8(op.opr1)) + int8_t(opr2) + int(CF);
			set8(op.opr1, setf8(val, dst > uint8_t(val)));
			return true;
		case 3: // sbb
			val = int8_t(dst = get8(op.opr1)) - int8_t(src = opr2 + int(CF));
			set8(op.opr1, setf8(val, dst < uint8_t(src)));
			return true;
		case 5: // sub
			val = int8_t(dst = get8(op.opr1)) - int8_t(src = opr2);
			set8(op.opr1, setf8(val, dst < uint8_t(src)));
			return true;
		case 7: // cmp
			val = int8_t(dst = get8(op.opr1)) - int8_t(src = opr2);
			setf8(val, dst < uint8_t(src));
			return true;
		}
		break;
	case 0x83: // r/m, imm16(signed)
		switch ((text[oldip + 1] >> 3) & 7) {
		case 0: // add
			val = int16_t(dst = get16(op.opr1)) + int8_t(opr2);
			set16(op.opr1, setf16(val, dst > uint8_t(val)));
			return true;
		case 2: // adc
			val = int16_t(dst = get16(op.opr1)) + int8_t(opr2) + int(CF);
			set16(op.opr1, setf16(val, dst > uint8_t(val)));
			return true;
		case 3: // sbb
			val = int8_t(dst = get8(op.opr1)) - int8_t(src = opr2 + int(CF));
			set8(op.opr1, setf8(val, dst < uint8_t(src)));
			return true;
		case 5: // sub
			val = int16_t(dst = get16(op.opr1)) - int8_t(src = opr2);
			set16(op.opr1, setf16(val, dst < uint16_t(src)));
			return true;
		case 7: // cmp
			val = int16_t(dst = get16(op.opr1)) - int8_t(src = opr2);
			setf16(val, dst < uint16_t(src));
			return true;
		}
		break;
	case 0x84: // test r/m, reg8
		setf8(int8_t(get8(op.opr1) & *r8[opr2]), false);
		return true;
	case 0x85: // test r/m, reg16
		setf16(int16_t(get16(op.opr1) & r[opr2]), false);
		return true;
	case 0x88: // mov r/m, reg8
		set8(op.opr1, *r8[opr2]);
		return true;
	case 0x89: // mov r/m, reg16
		set16(op.opr1, r[opr2]);
		return true;
	case 0x8a: // mov reg8, r/m
		*r8[opr1] = get8(op.opr2);
		return true;
	case 0x8b: // mov reg16, r/m
		r[opr1] = get16(op.opr2);
		return true;
	case 0x8d: // lea reg16, r/m
		r[opr1] = addr(op.opr2);
		return true;
	case 0xa0: // mov al, [addr]
		AL = get8(op.opr2);
		return true;
	case 0xa1: // mov ax, [addr]
		AX = get16(op.opr2);
		return true;
	case 0xa2: // mov [addr], al
		set8(op.opr2, AL);
		return true;
	case 0xa3: // mov [addr], ax
		set16(op.opr2, AX);
		return true;
	case 0xa8: // test al, imm8
		setf8(int8_t(AL & opr2), false);
		return true;
	case 0xa9: // test ax, imm16
		setf16(int16_t(AX & opr2), false);
		return true;
	case 0xb0: // mov reg8, imm8
	case 0xb1:
	case 0xb2:
	case 0xb3:
	case 0xb4:
	case 0xb5:
	case 0xb6:
	case 0xb7:
		*r8[opr1] = opr2;
		return true;
	case 0xb8: // mov reg16, imm16
	case 0xb9:
	case 0xba:
	case 0xbb:
	case 0xbc:
	case 0xbd:
	case 0xbe:
	case 0xbf:
		r[opr1] = opr2;
		return true;
	case 0xc3: // ret
		if (SP == 0) return false;
		break;
	case 0xc6: // mov r/m, imm8
		set8(op.opr1, opr2);
		return true;
	case 0xc7: // mov r/m, imm16
		set16(op.opr1, opr2);
		return true;
	case 0xf6:
		switch ((mem[1] >> 3) & 7) {
		case 0: // test r/m, imm8
			setf8(int8_t(get8(op.opr1) & opr2), false);
			return true;
		case 2: // not byte r/m
		case 3: // neg byte r/m
		case 4: // mul byte r/m
		case 5: // imul byte r/m
		case 6: // div byte r/m
		case 7: // idiv byte r/m
			break;
		}
		break;
	case 0xf7:
		switch ((mem[1] >> 3) & 7) {
		case 0: // test r/m, imm16
			setf16(int16_t(get16(op.opr1) & opr2), false);
			return true;
		case 2: // not r/m
		case 3: // neg r/m
		case 4: // mul r/m
		case 5: // imul r/m
		case 6: // div r/m
		case 7: // idiv r/m
			break;
		}
		break;
	}
	fprintf(stderr, "not implemented\n");
	return false;
}

static void run() {
	init_table();
	fprintf(stderr, header);
	while (run1());
}
	
int main(int argc, char *argv[]) {
	bool dis = false;
	const char *file = NULL;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-d")) {
			dis = true;
		} else if (i + 1 == argc) {
			file = argv[i];
		} else {
			break;
		}
	}
	if (!file) {
		fprintf(stderr, "usage: %s [-d] binary\n", argv[0]);
		return 1;
	}
	struct stat st;
	if (stat(file, &st)) {
		fprintf(stderr, "can not stat: %s\n", file);
		return 1;
	}
	size_t size = st.st_size;
	FILE *f = fopen(file, "rb");
	if (!f) {
		fprintf(stderr, "can not open: %s\n", file);
		return 1;
	}
	if (size >= 0x20) {
		uint8_t h[0x20];
		if (fread(h, sizeof(h), 1, f) && h[0] == 1 && h[1] == 3
			&& !fseek(f, h[4], SEEK_SET)) {
			if (h[3] != 4) {
				fprintf(stderr, "unknown cpu id: %d\n", h[3]);
				fclose(f);
				return 1;
			}
			size = read32(h + 8);
			int dsize = read32(h + 12);
			ip = read32(h + 20);
			if (h[2] & 0x20) {
				data = mem;
				fread(text, 1,  size, f);
				fread(data, 1, dsize, f);
			} else {
				data = text;
				fread(text, 1, size + dsize, f);
			}
		} else {
			fseek(f, 0, SEEK_SET);
		}
	}
	if (!data) {
		if (size > sizeof(text)) {
			fprintf(stderr, "too long raw binary: %s\n", file);
			fclose(f);
			return 1;
		}
		data = text;
		fread(text, 1, size, f);
	}
	fclose(f);
	if (dis) {
		disasm(text, size);
	} else {
		run();
	}
	return 0;
}

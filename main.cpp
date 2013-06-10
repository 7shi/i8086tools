#include "disasm.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

uint16_t ip, r[8];
uint8_t *r8[8];
uint8_t text[65536], mem[65536], *data;
bool OF, SF, ZF, CF;

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

const char *header = " AX   CX   DX   BX   SP   BP   SI   DI  FLAGS IP\n";

static void debug() {
	fprintf(stderr,
		"%04x %04x %04x %04x %04x %04x %04x %04x %c%c%c%c %04x",
		r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7],
		OF ? 'O' : '-', SF ? 'S' : '-', ZF ? 'Z' : '-', CF ? 'C' : '-', ip);
}

static void init_r8() {
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
}

inline uint16_t read16(uint16_t addr) {
	return data[addr] | (data[addr + 1] << 8);
}

inline void write16(uint16_t addr, uint16_t value) {
	data[addr] = value;
	data[addr + 1] = value >> 8;
}

static uint8_t get8(const Operand &opr) {
	switch (opr.type) {
	case Reg: return *r8[opr.value];
	case Imm: return opr.value;
	case Ptr: return data[opr.value];
	case ModRM + 0: return data[BX + SI + opr.value];
	case ModRM + 1: return data[BX + DI + opr.value];
	case ModRM + 2: return data[BP + SI + opr.value];
	case ModRM + 3: return data[BP + DI + opr.value];
	case ModRM + 4: return data[     SI + opr.value];
	case ModRM + 5: return data[     DI + opr.value];
	case ModRM + 6: return data[BP      + opr.value];
	case ModRM + 7: return data[BX      + opr.value];
	}
	return 0;
}

static uint16_t get16(const Operand &opr) {
	switch (opr.type) {
	case Reg: return r[opr.value];
	case Imm: return opr.value;
	case Ptr: return read16(opr.value);
	case ModRM + 0: return read16(BX + SI + opr.value);
	case ModRM + 1: return read16(BX + DI + opr.value);
	case ModRM + 2: return read16(BP + SI + opr.value);
	case ModRM + 3: return read16(BP + DI + opr.value);
	case ModRM + 4: return read16(     SI + opr.value);
	case ModRM + 5: return read16(     DI + opr.value);
	case ModRM + 6: return read16(BP      + opr.value);
	case ModRM + 7: return read16(BX      + opr.value);
	}
	return 0;
}

static void set8(const Operand &opr, uint8_t value) {
	switch (opr.type) {
	case Reg: *r8[opr.value] = value; return;
	case Ptr: data[opr.value] = value; return;
	case ModRM + 0: data[BX + SI + opr.value] = value; return;
	case ModRM + 1: data[BX + DI + opr.value] = value; return;
	case ModRM + 2: data[BP + SI + opr.value] = value; return;
	case ModRM + 3: data[BP + DI + opr.value] = value; return;
	case ModRM + 4: data[     SI + opr.value] = value; return;
	case ModRM + 5: data[     DI + opr.value] = value; return;
	case ModRM + 6: data[BP      + opr.value] = value; return;
	case ModRM + 7: data[BX      + opr.value] = value; return;
	}
}

static void set16(const Operand &opr, uint16_t value) {
	switch (opr.type) {
	case Reg: r[opr.value] = value; return;
	case Ptr: write16(opr.value, value); return;
	case ModRM + 0: write16(BX + SI + opr.value, value); return;
	case ModRM + 1: write16(BX + DI + opr.value, value); return;
	case ModRM + 2: write16(BP + SI + opr.value, value); return;
	case ModRM + 3: write16(BP + DI + opr.value, value); return;
	case ModRM + 4: write16(     SI + opr.value, value); return;
	case ModRM + 5: write16(     DI + opr.value, value); return;
	case ModRM + 6: write16(BP      + opr.value, value); return;
	case ModRM + 7: write16(BX      + opr.value, value); return;
	}
}

inline int setf8(int value, bool cf) {
	int8_t v = value;
	OF = value != v;
	SF = v < 0;
	ZF = v == 0;
	CF = cf;
	return value;
}

inline int setf16(int value, bool cf) {
	int16_t v = value;
	OF = value != v;
	SF = v < 0;
	ZF = v == 0;
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
	int dst, val;
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
	case 0x80: // r/m, imm8
		switch ((text[oldip + 1] >> 3) & 7) {
		case 0: // add
			val = int8_t(dst = get8(op.opr1)) + int8_t(opr2);
			set8(op.opr1, setf8(val, dst > uint8_t(val)));
			break;
		}
		return true;
	case 0x81: // r/m, imm16
		switch ((text[oldip + 1] >> 3) & 7) {
		case 0: // add
			val = int16_t(dst = get16(op.opr1)) + int16_t(opr2);
			set16(op.opr1, setf16(val, dst > uint16_t(val)));
			break;
		}
		return true;
	case 0x82: // r/m, imm8(signed)
		switch ((text[oldip + 1] >> 3) & 7) {
		case 0: // add
			val = int8_t(dst = get8(op.opr1)) + int8_t(opr2);
			set8(op.opr1, setf8(val, dst > uint8_t(val)));
			break;
		}
		return true;
	case 0x83: // r/m, imm16(signed)
		switch ((text[oldip + 1] >> 3) & 7) {
		case 0: // add
			val = int16_t(dst = get16(op.opr1)) + int8_t(opr2);
			set16(op.opr1, setf16(val, dst > uint8_t(val)));
			break;
		}
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
	}
	fprintf(stderr, "not implemented\n");
	return false;
}

static void run() {
	init_r8();
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

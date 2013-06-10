#include "disasm.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

uint16_t ip, r[8];
uint8_t *r8[8];
uint8_t text[65536], mem[65536], *data;

#define SP r[4]

static void debug() {
	fprintf(stderr,
		"%04x %04x %04x %04x %04x %04x %04x %04x %04x",
		r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], ip);
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

static bool run1() {
	OpCode op = disasm1(text + ip, ip);
	std::string hex = hexdump(text + ip, op.len);
	debug();
	fprintf(stderr, ":%-12s %s\n", hex.c_str(), op.str().c_str());
	uint8_t b = text[ip];
	uint16_t oldip = ip;
	ip += op.len;
	switch (b) {
	case 0x50: // push reg16
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
		SP -= 2;
		write16(SP, r[op.opr1.value]);
		return true;
	case 0x58: // pop reg16
	case 0x59:
	case 0x5a:
	case 0x5b:
	case 0x5c:
	case 0x5d:
	case 0x5e:
	case 0x5f:
		r[op.opr1.value] = read16(SP);
		SP += 2;
		return true;
	case 0xb0: // mov reg8, imm8
	case 0xb1:
	case 0xb2:
	case 0xb3:
	case 0xb4:
	case 0xb5:
	case 0xb6:
	case 0xb7:
		*r8[op.opr1.value] = op.opr2.value;
		return true;
	case 0xb8: // mov reg16, imm16
	case 0xb9:
	case 0xba:
	case 0xbb:
	case 0xbc:
	case 0xbd:
	case 0xbe:
	case 0xbf:
		r[op.opr1.value] = op.opr2.value;
		return true;
	case 0xc3: // ret
		if (SP == 0) return false;
		break;
	}
	fprintf(stderr, "not implemented\n");
	return false;
}

static void run() {
	init_r8();
	fprintf(stderr, " AX   CX   DX   BX   SP   BP   SI   DI   IP\n");
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

#include "VM.h"
#include "disasm.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

const char *header = " AX   CX   DX   BX   SP   BP   SI   DI  FLAGS  IP\n";
bool ptable[256];
bool verbose;

static bool initialized;

static void init_table() {
	for (int i = 0; i < 256; i++) {
		int n = 0;
		for (int j = 1; j < 256; j += j) {
			if (i & j) n++;
		}
		ptable[i] = (n & 1) == 0;
	}
	initialized = true;
}

VM::VM(): ip(0), data(NULL), tsize(0), start_sp(0), exit_status(0) {
	if (!initialized) init_table();
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
	memset(r, 0, sizeof(r));
	memset(text, 0, sizeof(text));
	memset(mem, 0, sizeof(mem));
	OF = DF = SF = ZF = PF = CF = false;
}

void VM::debug(const OpCode &op) {
	fprintf(stderr,
		"%04x %04x %04x %04x %04x %04x %04x %04x %c%c%c%c%c %04x:%-12s %s\n",
		r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7],
		OF?'O':'-', SF?'S':'-', ZF?'Z':'-', PF?'P':'-', CF?'C':'-', ip,
		hexdump(text + ip, op.len).c_str(), op.str().c_str());
}

int VM::addr(const Operand &opr) {
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

uint8_t VM::get8(const Operand &opr) {
	switch (opr.type) {
	case Reg: return *r8[opr.value];
	case Imm: return opr.value;
	}
	int ad = addr(opr);
	return ad < 0 ? 0 : data[ad];
}

uint16_t VM::get16(const Operand &opr) {
	switch (opr.type) {
	case Reg: return r[opr.value];
	case Imm: return opr.value;
	}
	int ad = addr(opr);
	return ad < 0 ? 0 : read16(ad);
}

void VM::set8(const Operand &opr, uint8_t value) {
	if (opr.type == Reg) {
		*r8[opr.value] = value;
	} else {
		int ad = addr(opr);
		if (ad >= 0) data[ad] = value;
	}
}

void VM::set16(const Operand &opr, uint16_t value) {
	if (opr.type == Reg) {
		r[opr.value] = value;
	} else {
		int ad = addr(opr);
		if (ad >= 0) write16(ad, value);
	}
}

void VM::run(const std::vector<std::string> &args) {
	int arglen = 0;
	for (int i = 0; i < (int)args.size(); i++) {
		arglen += args[i].size() + 1;
	}
	SP -= (arglen + 1) & ~1;
	uint16_t ad1 = SP;
	write16(SP -= 2, 0); // envp[0]
	write16(SP -= 2, 0); // argv[argc]
	SP -= 2 * args.size();
	for (int i = 0; i < (int)args.size(); i++) {
		write16(SP + 2 * i, ad1);
		strcpy((char *)data + ad1, args[i].c_str());
		ad1 += args[i].size() + 1;
	}
	write16(SP -= 2, args.size()); // argc
	start_sp = SP;
	if (verbose) fprintf(stderr, header);
	hasExited = false;
	while (!hasExited) run1();
}

bool VM::read(const char *file) {
	struct stat st;
	if (stat(file, &st)) {
		fprintf(stderr, "can not stat: %s\n", file);
		return false;
	}
	tsize = st.st_size;
	FILE *f = fopen(file, "rb");
	if (!f) {
		fprintf(stderr, "can not open: %s\n", file);
		return false;
	}
	if (tsize >= 0x20) {
		uint8_t h[0x20];
		if (fread(h, sizeof(h), 1, f) && h[0] == 1 && h[1] == 3
			&& !fseek(f, h[4], SEEK_SET)) {
			if (h[3] != 4) {
				fprintf(stderr, "unknown cpu id: %d\n", h[3]);
				fclose(f);
				return 1;
			}
			tsize = read32(h + 8);
			int dsize = read32(h + 12);
			ip = read32(h + 20);
			if (h[2] & 0x20) {
				data = mem;
				fread(text, 1, tsize, f);
				fread(data, 1, dsize, f);
			} else {
				data = text;
				fread(text, 1, tsize + dsize, f);
			}
		} else {
			fseek(f, 0, SEEK_SET);
		}
	}
	if (!data) {
		if (tsize > sizeof(text)) {
			fprintf(stderr, "too long raw binary: %s\n", file);
			fclose(f);
			return 1;
		}
		data = text;
		fread(text, 1, tsize, f);
	}
	fclose(f);
	return true;
}

void VM::disasm() {
	::disasm(text, tsize);
}

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <vector>
#include <string>

std::string regs8 [] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
std::string regs16[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
std::string sregs [] = { "es", "cs", "ss", "ds" };

std::string hex(uint16_t v) {
	char buf[8];
	snprintf(buf, sizeof(buf), "%04x", v);
	return buf;
}

uint16_t disp8(const std::vector<uint8_t> &mem, off_t index) {
	return index + 1 + (int8_t)mem.at(index);
}

struct OpCode {
	size_t len;
	std::string mne, op1, op2;

	OpCode(
		int len = 0,
		const std::string &mne = "",
		const std::string &op1 = "",
		const std::string &op2 = "")
		: len(len), mne(mne), op1(op1), op2(op2) {}
	OpCode(int len, const std::string &mne, uint16_t addr)
		: len(len), mne(mne), op1(hex(addr)) {}

	inline bool empty() { return len == 0; }

	std::string str() {
		if (op1.empty()) return mne;
		if (op2.empty()) return mne + " " + op1;
		return mne + " " + op1 + ", " + op2;
	}
};

int undefined;

OpCode disasm(const std::vector<uint8_t> &mem, off_t index) {
	uint8_t b = mem.at(index);
	switch (b) {
	case 0x27: return OpCode(1, "baa");
	case 0x2f: return OpCode(1, "das");
	case 0x37: return OpCode(1, "aaa");
	case 0x3f: return OpCode(1, "aas");
	case 0x70: return OpCode(2, "jo"  , disp8(mem, index + 1));
	case 0x71: return OpCode(2, "jno" , disp8(mem, index + 1));
	case 0x72: return OpCode(2, "jb"  , disp8(mem, index + 1));
	case 0x73: return OpCode(2, "jnb" , disp8(mem, index + 1));
	case 0x74: return OpCode(2, "je"  , disp8(mem, index + 1));
	case 0x75: return OpCode(2, "jne" , disp8(mem, index + 1));
	case 0x76: return OpCode(2, "jbe" , disp8(mem, index + 1));
	case 0x77: return OpCode(2, "jnbe", disp8(mem, index + 1));
	case 0x78: return OpCode(2, "js"  , disp8(mem, index + 1));
	case 0x79: return OpCode(2, "jns" , disp8(mem, index + 1));
	case 0x7a: return OpCode(2, "jp"  , disp8(mem, index + 1));
	case 0x7b: return OpCode(2, "jnp" , disp8(mem, index + 1));
	case 0x7c: return OpCode(2, "jl"  , disp8(mem, index + 1));
	case 0x7d: return OpCode(2, "jnl" , disp8(mem, index + 1));
	case 0x7e: return OpCode(2, "jle" , disp8(mem, index + 1));
	case 0x7f: return OpCode(2, "jnle", disp8(mem, index + 1));
	case 0x98: return OpCode(1, "cbw");
	case 0x99: return OpCode(1, "cwd");
	case 0x90: return OpCode(1, "nop");
	case 0x9b: return OpCode(1, "wait");
	case 0x9c: return OpCode(1, "pushf");
	case 0x9d: return OpCode(1, "popf");
	case 0x9e: return OpCode(1, "sahf");
	case 0x9f: return OpCode(1, "lahf");
	case 0xa4: return OpCode(1, "movsb");
	case 0xa5: return OpCode(1, "movsw");
	case 0xa6: return OpCode(1, "cmpsb");
	case 0xa7: return OpCode(1, "cmpsw");
	case 0xaa: return OpCode(1, "stosb");
	case 0xab: return OpCode(1, "stosw");
	case 0xac: return OpCode(1, "lodsb");
	case 0xad: return OpCode(1, "lodsw");
	case 0xae: return OpCode(1, "scansb");
	case 0xaf: return OpCode(1, "scansw");
	case 0xc3: return OpCode(1, "ret");
	case 0xcb: return OpCode(1, "retf");
	case 0xcc: return OpCode(1, "int3");
	case 0xce: return OpCode(1, "into");
	case 0xcf: return OpCode(1, "iret");
	case 0xd4: if (mem.at(1) == 0x0a) return OpCode(2, "aam"); else break;
	case 0xd5: if (mem.at(1) == 0x0a) return OpCode(2, "aad"); else break;
	case 0xd7: return OpCode(1, "xlat");
	case 0xe0: return OpCode(2, "loopnz"   , disp8(mem, index + 1));
	case 0xe1: return OpCode(2, "loopz"    , disp8(mem, index + 1));
	case 0xe2: return OpCode(2, "loop"     , disp8(mem, index + 1));
	case 0xe3: return OpCode(2, "jcxz"     , disp8(mem, index + 1));
	case 0xeb: return OpCode(2, "jmp short", disp8(mem, index + 1));
	case 0xec: return OpCode(1, "in", "al", "dx");
	case 0xed: return OpCode(1, "in", "ax", "dx");
	case 0xee: return OpCode(1, "out", "dx", "al");
	case 0xef: return OpCode(1, "out", "dx", "ax");
	case 0xf0: return OpCode(1, "lock");
	case 0xf2: return OpCode(1, "repnz");
	case 0xf3: return OpCode(1, "repz");
	case 0xf4: return OpCode(1, "hlt");
	case 0xf5: return OpCode(1, "cmc");
	case 0xf8: return OpCode(1, "clc");
	case 0xf9: return OpCode(1, "stc");
	case 0xfa: return OpCode(1, "cli");
	case 0xfb: return OpCode(1, "sti");
	case 0xfc: return OpCode(1, "cld");
	case 0xfd: return OpCode(1, "std");
	}
	switch (b & ~7) {
	case 0x40: return OpCode(1, "inc" , regs16[b & 7]);
	case 0x48: return OpCode(1, "dec" , regs16[b & 7]);
	case 0x50: return OpCode(1, "push", regs16[b & 7]);
	case 0x58: return OpCode(1, "pop" , regs16[b & 7]);
	case 0x90: return OpCode(1, "xchg", regs16[b & 7], "ax");
	}
	if (b != 0x0f) {
		switch (b & 0xe7) {
			case 0x06: return OpCode(1, "push", sregs[(b >> 3) & 3]);
			case 0x07: return OpCode(1, "pop" , sregs[(b >> 3) & 3]);
			case 0x26: return OpCode(1, sregs[(b >> 3) & 3] + ":");
		}
	}
	undefined++;
	return OpCode(1, "(undefined)");
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s binary\n", argv[0]);
		return 1;
	}
	struct stat st;
	if (stat(argv[1], &st)) {
		fprintf(stderr, "can not stat: %s\n", argv[1]);
		return 1;
	}
	FILE *f = fopen(argv[1], "rb");
	if (!f) {
		fprintf(stderr, "can not open: %s\n", argv[1]);
		return 1;
	}
	std::vector<uint8_t> aout(st.st_size);
	fread(&aout[0], 1, st.st_size, f);
	fclose(f);

	off_t index = 0;
	while (index < aout.size()) {
		OpCode op = disasm(aout, index);
		std::string hex;
		char buf[3];
		for (int i = 0; i < op.len; i++) {
			snprintf(buf, sizeof(buf), "%02x", aout[index + i]);
			hex += buf;
		}
		printf("%04x: %-10s %s\n", index, hex.c_str(), op.str().c_str());
		index += op.len;
	}
	printf("undefined: %d\n", undefined);

	return 0;
}

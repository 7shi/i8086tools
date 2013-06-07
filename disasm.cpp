#include "disasm.h"
#include <stdio.h>
#include <string.h>

std::string aregs [] = { "al", "ax" };
std::string regs8 [] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
std::string regs16[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
std::string sregs [] = { "es", "cs", "ss", "ds" };

std::string hex(int v, int len) {
	char buf[16];
	if (len == 0) {
		strcpy(buf, "%x");
	} else {
		snprintf(buf, sizeof(buf), "%%0%dx", len);
	}
	std::string format = buf;
	snprintf(buf, sizeof(buf), format.c_str(), v);
	return buf;
}

std::string readfar(uint8_t *mem) {
	return hex(read16(mem), 4) + ":" + hex(read16(mem + 2), 4);
}

OpCode::OpCode(
	int len, const std::string &mne, const std::string &op1, const std::string &op2)
	: prefix(false), len(len), mne(mne), op1(op1), op2(op2) {}
OpCode::OpCode(
	int len, const std::string &mne, uint16_t value, const std::string &op2)
	: prefix(false), len(len), mne(mne), op1(hex(value, 4)), op2(op2) {}
OpCode::OpCode(
	int len, const std::string &mne, const std::string &op1, uint16_t value)
	: prefix(false), len(len), mne(mne), op1(op1), op2(hex(value, 4)) {}
OpCode::OpCode(
	const std::string &mne)
	: prefix(true), len(1), mne(mne) {}

std::string OpCode::str() {
	if (op1.empty()) return mne;
	if (op2.empty()) return mne + " " + op1;
	return mne + " " + op1 + ", " + op2;
}

static OpCode modrm(uint8_t *mem, const std::string &mne, bool w) {
	OpCode ret(2, mne);
	uint8_t b = mem[1], mod = b >> 6, rm = b & 7;
	int16_t disp = 0;
	switch (mod) {
	case 1:
		ret.len++;
		disp = (int8_t)mem[2];
		break;
	case 2:
		ret.len += 2;
		disp = read16(&mem[2]);
		break;
	case 3:
		ret.op1 = w ? regs16[rm] : regs8[rm];
		return ret;
	}
	if (!w) ret.mne += " byte";
	ret.op1 = "[";
	if (mod == 0 && rm == 6) {
		ret.len += 2;
		ret.op1 += hex(read16(&mem[2]), 4);
	} else {
		const char *rms[] = { "bx+si", "bx+di", "bp+si", "bp+di", "si", "di", "bp", "bx" };
		ret.op1 += rms[rm];
		if (disp > 0) {
			ret.op1 += "+" + hex(disp);
		} else if (disp < 0) {
			ret.op1 += "-" + hex(-disp);
		}
	}
	ret.op1 += "]";
	return ret;
}

static OpCode regrm(uint8_t *mem, const std::string &mne, bool d, int w) {
	OpCode op = modrm(mem, mne, w);
	int reg = (mem[1] >> 3) & 7;
	std::string r = w == 0 ? regs8[reg] : w == 1 ? regs16[reg] : sregs[reg & 3];
	return d ?
		OpCode(op.len, mne, r, op.op1):
		OpCode(op.len, mne, op.op1, r);
}

static int undefined;

OpCode disasm1(uint8_t *mem, uint16_t addr) {
	uint8_t b = mem[0];
	switch (b) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03: return regrm(mem, "add", b & 2, b & 1);
	case 0x04: return OpCode(2, "add", "al", hex(mem[1]));
	case 0x05: return OpCode(3, "add", "ax", read16(mem + 1));
	case 0x06: return OpCode(1, "push", "es");
	case 0x07: return OpCode(1, "pop" , "es");
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b: return regrm(mem, "or", b & 2, b & 1);
	case 0x0c: return OpCode(2, "or" , "al", hex(mem[1]));
	case 0x0d: return OpCode(3, "or" , "ax", read16(mem + 1));
	case 0x0e: return OpCode(1, "push", "cs");
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13: return regrm(mem, "adc", b & 2, b & 1);
	case 0x14: return OpCode(2, "adc", "al", hex(mem[1]));
	case 0x15: return OpCode(3, "adc", "ax", read16(mem + 1));
	case 0x16: return OpCode(1, "push", "ss");
	case 0x17: return OpCode(1, "pop" , "ss");
	case 0x18:
	case 0x19:
	case 0x1a:
	case 0x1b: return regrm(mem, "ssb", b & 2, b & 1);
	case 0x1c: return OpCode(2, "ssb", "al", hex(mem[1]));
	case 0x1d: return OpCode(3, "ssb", "ax", read16(mem + 1));
	case 0x1e: return OpCode(1, "push", "ds");
	case 0x1f: return OpCode(1, "pop" , "ds");
	case 0x20:
	case 0x21:
	case 0x22:
	case 0x23: return regrm(mem, "and", b & 2, b & 1);
	case 0x24: return OpCode(2, "and", "al", hex(mem[1]));
	case 0x25: return OpCode(3, "and", "ax", read16(mem + 1));
	case 0x26: return OpCode("es:");
	case 0x27: return OpCode(1, "daa");
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b: return regrm(mem, "sub", b & 2, b & 1);
	case 0x2c: return OpCode(2, "sub", "al", hex(mem[1]));
	case 0x2d: return OpCode(3, "sub", "ax", read16(mem + 1));
	case 0x2e: return OpCode("cs:");
	case 0x2f: return OpCode(1, "das");
	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33: return regrm(mem, "xor", b & 2, b & 1);
	case 0x34: return OpCode(2, "xor", "al", hex(mem[1]));
	case 0x35: return OpCode(3, "xor", "ax", read16(mem + 1));
	case 0x36: return OpCode("ss:");
	case 0x37: return OpCode(1, "aaa");
	case 0x38:
	case 0x39:
	case 0x3a:
	case 0x3b: return regrm(mem, "cmp", b & 2, b & 1);
	case 0x3c: return OpCode(2, "cmp", "al", hex(mem[1]));
	case 0x3d: return OpCode(3, "cmp", "ax", read16(mem + 1));
	case 0x3e: return OpCode("ds:");
	case 0x3f: return OpCode(1, "aas");
	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47: return OpCode(1, "inc" , regs16[b & 7]);
	case 0x48:
	case 0x49:
	case 0x4a:
	case 0x4b:
	case 0x4c:
	case 0x4d:
	case 0x4e:
	case 0x4f: return OpCode(1, "dec" , regs16[b & 7]);
	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57: return OpCode(1, "push", regs16[b & 7]);
	case 0x58:
	case 0x59:
	case 0x5a:
	case 0x5b:
	case 0x5c:
	case 0x5d:
	case 0x5e:
	case 0x5f: return OpCode(1, "pop" , regs16[b & 7]);
	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	case 0x76:
	case 0x77:
	case 0x78:
	case 0x79:
	case 0x7a:
	case 0x7b:
	case 0x7c:
	case 0x7d:
	case 0x7e:
	case 0x7f: {
		const char *mnes[] = {
			"jo", "jno", "jb", "jnb", "je", "jne", "jbe", "jnbe",
			"js", "jns", "jp", "jnp", "jl", "jnl", "jle", "jnle" };
		return OpCode(2, mnes[b & 15], disp8(mem + 1, addr + 1));
	}
	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83: {
		const char *mnes[] = { "add", "or", "adc", "ssb", "and", "sub", "xor", "cmp" };
		std::string mne = mnes[(mem[1] >> 3) & 7];
		OpCode op = modrm(mem, mne, b & 1);
		off_t iimm = op.len;
		if (b & 2) {
			op.len++;
			int8_t v = (int8_t)mem[iimm];
			op.op2 = v >= 0 ? hex(v) : "-" + hex(-v);
		} else if (b & 1) {
			op.len += 2;
			op.op2 = hex(read16(mem + iimm), 4);
		} else {
			op.len++;
			op.op2 = hex(mem[iimm]);
		}
		return op;
	}
	case 0x84:
	case 0x85: return regrm(mem, "test", true, b & 1);
	case 0x86:
	case 0x87: return regrm(mem, "xchg", true, b & 1);
	case 0x88:
	case 0x89:
	case 0x8a:
	case 0x8b: return regrm(mem, "mov", b & 2, b & 1);
	case 0x8c: return regrm(mem, "mov", false, 2);
	case 0x8d: return regrm(mem, "lea", true , 1);
	case 0x8e: return regrm(mem, "mov", true , 2);
	case 0x8f: return modrm(mem, "pop", true);
	case 0x90: return OpCode(1, "nop");
	case 0x91:
	case 0x92:
	case 0x93:
	case 0x94:
	case 0x95:
	case 0x96:
	case 0x97: return OpCode(1, "xchg", regs16[b & 7], "ax");
	case 0x98: return OpCode(1, "cbw");
	case 0x99: return OpCode(1, "cwd");
	case 0x9a: return OpCode(5, "callf", readfar(mem + 1));
	case 0x9b: return OpCode(1, "wait");
	case 0x9c: return OpCode(1, "pushf");
	case 0x9d: return OpCode(1, "popf");
	case 0x9e: return OpCode(1, "sahf");
	case 0x9f: return OpCode(1, "lahf");
	case 0xa0:
	case 0xa1: return OpCode(3, "mov", aregs[b & 1], "[" + hex(read16(mem + 1), 4) + "]");
	case 0xa2: return OpCode(3, "mov", "[" + hex(read16(mem + 1), 4) + "]", "al");
	case 0xa3: return OpCode(3, "mov", "[" + hex(read16(mem + 1), 4) + "]", "ax");
	case 0xa4: return OpCode(1, "movsb");
	case 0xa5: return OpCode(1, "movsw");
	case 0xa6: return OpCode(1, "cmpsb");
	case 0xa7: return OpCode(1, "cmpsw");
	case 0xa8: return OpCode(2, "test", "al", hex(mem[1]));
	case 0xa9: return OpCode(3, "test", "ax", read16(mem + 1));
	case 0xaa: return OpCode(1, "stosb");
	case 0xab: return OpCode(1, "stosw");
	case 0xac: return OpCode(1, "lodsb");
	case 0xad: return OpCode(1, "lodsw");
	case 0xae: return OpCode(1, "scasb");
	case 0xaf: return OpCode(1, "scasw");
	case 0xb0:
	case 0xb1:
	case 0xb2:
	case 0xb3:
	case 0xb4:
	case 0xb5:
	case 0xb6:
	case 0xb7: return OpCode(2, "mov", regs8[b & 7], hex(mem[1]));
	case 0xb8:
	case 0xb9:
	case 0xba:
	case 0xbb:
	case 0xbc:
	case 0xbd:
	case 0xbe:
	case 0xbf: return OpCode(3, "mov", regs16[b & 7], read16(mem + 1));
	case 0xc2: return OpCode(3, "ret", read16(mem + 1));
	case 0xc3: return OpCode(1, "ret");
	case 0xc4: return regrm(mem, "les", true, 1);
	case 0xc5: return regrm(mem, "lds", true, 1);
	case 0xc6:
	case 0xc7: {
		OpCode op = modrm(mem, "mov", b & 1);
		off_t iimm = op.len;
		return b & 1 ?
			OpCode(op.len + 2, op.mne, op.op1, read16(mem + iimm)):
			OpCode(op.len + 1, op.mne, op.op1, hex(mem[iimm]));
	}
	case 0xca: return OpCode(3, "retf", read16(mem + 1));
	case 0xcb: return OpCode(1, "retf");
	case 0xcc: return OpCode(1, "int3");
	case 0xcd: return OpCode(2, "int", hex(mem[1]));
	case 0xce: return OpCode(1, "into");
	case 0xcf: return OpCode(1, "iret");
	case 0xd0:
	case 0xd1:
	case 0xd2:
	case 0xd3: {
		const char *mnes[] = { "rol", "ror", "rcl", "rcr", "shl", "shr", "", "sar" };
		std::string mne = mnes[(mem[1] >> 3) & 7];
		if (mne.empty()) break;
		OpCode op = modrm(mem, mne, b & 1);
		return OpCode(op.len, op.mne, op.op1, b & 2 ? "cl" : "1");
	}
	case 0xd4: if (mem[1] == 0x0a) return OpCode(2, "aam"); else break;
	case 0xd5: if (mem[1] == 0x0a) return OpCode(2, "aad"); else break;
	case 0xd7: return OpCode(1, "xlat");
#if 0
	case 0xd8:
	case 0xd9:
	case 0xda:
	case 0xdb:
	case 0xdc:
	case 0xdd:
	case 0xde:
	case 0xdf: {
		OpCode op = modrm(mem, "esc", false);
		int code = ((b & 7) << 8) | ((mem[1] >> 3) & 7);
		return OpCode(op.len, op.mne, hex(code, 2), op.op1);
	}
#endif
	case 0xe0: return OpCode(2, "loopnz", disp8(mem + 1, addr + 1));
	case 0xe1: return OpCode(2, "loopz" , disp8(mem + 1, addr + 1));
	case 0xe2: return OpCode(2, "loop"  , disp8(mem + 1, addr + 1));
	case 0xe3: return OpCode(2, "jcxz"  , disp8(mem + 1, addr + 1));
	case 0xe4:
	case 0xe5: return OpCode(2, "in", aregs[b & 1], hex(mem[1]));
	case 0xe6:
	case 0xe7: return OpCode(2, "out", hex(mem[1]), aregs[b & 1]);
	case 0xe8: return OpCode(3, "call", disp16(mem + 1, addr + 1));
	case 0xe9: return OpCode(3, "jmp" , disp16(mem + 1, addr + 1));
	case 0xea: return OpCode(5, "jmpf", readfar(mem + 1));
	case 0xeb: return OpCode(2, "jmp short", disp8(mem + 1, addr + 1));
	case 0xec:
	case 0xed: return OpCode(1, "in", aregs[b & 1], "dx");
	case 0xee:
	case 0xef: return OpCode(1, "out", "dx", aregs[b & 1]);
	case 0xf0: return OpCode("lock");
	case 0xf2: return OpCode("repnz");
	case 0xf3: return OpCode("repz");
	case 0xf4: return OpCode(1, "hlt");
	case 0xf5: return OpCode(1, "cmc");
	case 0xf6:
	case 0xf7: {
		const char *mnes[] = { "test", "", "not", "neg", "mul", "imul", "div", "idiv" };
		int t = (mem[1] >> 3) & 7;
		std::string mne = mnes[t];
		if (mne.empty()) break;
		OpCode op = modrm(mem, mne, b & 1);
		if (t > 0) return op;
		off_t iimm = op.len;
		return b & 1 ?
			OpCode(op.len + 2, op.mne, op.op1, read16(mem + iimm)):
			OpCode(op.len + 1, op.mne, op.op1, hex(mem[iimm]));
	}
	case 0xf8: return OpCode(1, "clc");
	case 0xf9: return OpCode(1, "stc");
	case 0xfa: return OpCode(1, "cli");
	case 0xfb: return OpCode(1, "sti");
	case 0xfc: return OpCode(1, "cld");
	case 0xfd: return OpCode(1, "std");
	case 0xfe:
	case 0xff: {
		const char *mnes[] = { "inc", "dec", "call", "callf", "jmp", "jmpf", "push", "" };
		std::string mne = mnes[(mem[1] >> 3) & 7];
		if (mne.empty()) break;
		return modrm(mem, mne, b & 1);
	}}
	undefined++;
	return OpCode(1, "(undefined)");
}

OpCode disasm1(uint8_t *mem, uint16_t addr, size_t last) {
	OpCode op1 = disasm1(mem, addr);
	uint16_t addr2 = addr + op1.len;
	if (!op1.prefix || addr2 > last) return op1;

	OpCode op2 = disasm1(mem + op1.len, addr2);
	if (op2.prefix || addr2 + op2.len > last) return op1;

	op2.len += op1.len;
	if (op1.mne[op1.mne.size() - 1] == ':') {
		char f1 = op2.op1.size() < 1 ? 0 : op2.op1[0];
		char f2 = op2.op2.size() < 1 ? 0 : op2.op2[0];
		if (f1 == '[') {
			op2.op1.insert(1, op1.mne);
			return op2;
		} else if (f2 == '[') {
			op2.op2.insert(1, op1.mne);
			return op2;
		}
	}
	op2.mne = op1.mne + " " + op2.mne;
	return op2;
}

void disasm(uint8_t *mem, size_t size) {
	undefined = 0;
	off_t index = 0;
	while (index < size) {
		OpCode op = disasm1(mem + index, index, size);
		if (index + op.len > size) {
			op = OpCode(size - index, "db");
			for (; index < size; index++) {
				if (!op.op1.empty()) op.op1 += ", ";
				op.op1 += hex(mem[index], 2);
			}
		}
		std::string hex;
		char buf[3];
		for (int i = 0; i < op.len; i++) {
			snprintf(buf, sizeof(buf), "%02x", mem[index + i]);
			hex += buf;
		}
		printf("%04x: %-12s %s\n", index, hex.c_str(), op.str().c_str());
		index += op.len;
	}
	printf("undefined: %d\n", undefined);
}

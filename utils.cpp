#include "utils.h"
#include <stdio.h>
#include <string.h>

std::string regs8 [] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
std::string regs16[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
std::string sregs [] = { "es", "cs", "ss", "ds" };

Operand dx = Operand(0, true , Reg , 2);
Operand cl = Operand(0, false, Reg , 1);
Operand es = Operand(0, true , SReg, 0);
Operand cs = Operand(0, true , SReg, 1);
Operand ss = Operand(0, true , SReg, 2);
Operand ds = Operand(0, true , SReg, 3);

std::string hex(int v, int len) {
	char buf[32], format[16] = "%x";
	if (v < 0) {
		strcpy(format, "-%x");
		v = -v;
	} else if (0 < len && len < 32) {
		snprintf(format, sizeof(format), "%%0%dx", len);
	}
	snprintf(buf, sizeof(buf), format, v);
	return buf;
}

std::string hexdump(uint8_t *mem, int len) {
	std::string ret;
	char buf[3];
	for (int i = 0; i < len; i++) {
		snprintf(buf, sizeof(buf), "%02x", mem[i]);
		ret += buf;
	}
	return ret;
}

OpCode::OpCode()
	: prefix(false), len(0) {}
OpCode::OpCode(
	int len, const std::string &mne, const Operand &opr1, const Operand &opr2)
	: prefix(false), len(len), mne(mne), opr1(opr1), opr2(opr2) {}
OpCode::OpCode(const std::string &mne, const Operand &opr)
	: prefix(true), len(1), mne(mne), opr2(opr) {}

std::string OpCode::str() {
	if (opr1.empty()) return mne;
	std::string opr1s = opr1.str();
	if (opr1.type >= Ptr && !opr1.w && (opr2.empty() || opr2.type == Imm))
		opr1s = "byte " + opr1s;
	if (opr2.empty()) return mne + " " + opr1s;
	return mne + " " + opr1s + ", " + opr2.str();
}

void OpCode::swap() {
	Operand tmp = opr1;
	opr1 = opr2;
	opr2 = tmp;
}

Operand::Operand()
	: len(-1), w(false), type(0), value(0), seg(-1) {}
Operand::Operand(int len, bool w, int type, int value)
	: len(len), w(w), type(type), value(value), seg(-1) {}

std::string Operand::str() {
	switch (type) {
	case Reg : return (w ? regs16 : regs8)[value];
	case SReg: return sregs[value];
	case Imm : return hex(value, len == 2 ? 4 : 0);
	case Addr: return hex(value, 4);
	case Far : return hex((value >> 16) & 0xffff, 4) + ":" + hex(value & 0xffff, 4);
	}
	std::string ret = "[";
	if (seg >= 0) ret += sregs[seg] + ":";
	if (type == Ptr) return ret + hex(value, 4) + "]";
	const char *ms[] = { "bx+si", "bx+di", "bp+si", "bp+di", "si", "di", "bp", "bx" };
	ret += ms[type - ModRM];
	if (value > 0) {
		ret += "+" + hex(value);
	} else if (value < 0) {
		ret += hex(value);
	}
	return ret + "]";
}

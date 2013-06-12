#pragma once
#include "utils.h"
#include <string>

enum OperandType { Reg, SReg, Imm, Addr, Far, Ptr, ModRM };

struct Operand {
	int len;
	bool w;
	int type, value, seg;
	
	Operand();
	Operand(int len, bool w, int type, int value);

	inline bool empty() const { return len == -1; }
	std::string str() const;
};

extern Operand dx, cl, es, cs, ss, ds;

inline Operand reg(int r, bool w) { return Operand(0, w    , Reg, r); }
inline Operand imm16(uint16_t  v) { return Operand(2, true , Imm, v); }
inline Operand imm8 (uint8_t   v) { return Operand(1, false, Imm, v); }
inline Operand far  (uint32_t  a) { return Operand(4, false, Far, a); }
inline Operand ptr(uint16_t a, bool w) {
	return Operand(2, w, Ptr, a);
}
inline Operand disp8(uint8_t *mem, uint16_t addr) {
	return Operand(1, false, Addr, (uint16_t)(addr + 1 + (int8_t)mem[0]));
}
inline Operand disp16(uint8_t *mem, uint16_t addr) {
	return Operand(2, true, Addr, (uint16_t)(addr + 2 + (int16_t)read16(mem)));
}

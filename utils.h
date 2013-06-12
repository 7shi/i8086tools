#pragma once
#include <stdint.h>
#include <string>

extern std::string rootpath;

extern std::string regs8[];
extern std::string regs16[];
extern std::string sregs[];

inline uint16_t read16(uint8_t *mem) {
	return mem[0] | (mem[1] << 8);
}

inline uint32_t read32(uint8_t *mem) {
	return mem[0] | (mem[1] << 8) | (mem[2] << 16) | (mem[3] << 24);
}

extern std::string hex(int v, int len = 0);
extern std::string hexdump(uint8_t *mem, int len);
void setroot(std::string root);
std::string convpath(const std::string &path);
bool startsWith(const std::string &s, const std::string &prefix);
bool endsWith(const std::string &s, const std::string &suffix);
std::string replace(const std::string &src, const std::string &s1, const std::string &s2);

#ifdef WIN32
extern std::string getErrorMessage(int err);
#endif

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

struct OpCode {
	bool prefix;
	size_t len;
	std::string mne;
	Operand opr1, opr2;

	OpCode();
	OpCode(
		int len, const std::string &mne,
		const Operand &opr1 = Operand(),
		const Operand &opr2 = Operand());
	OpCode(const std::string &mne, const Operand &opr = Operand());

	inline bool empty() const { return len == 0; }
	std::string str() const;
	void swap();
};

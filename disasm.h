#pragma once
#include <stdint.h>
#include <string>

extern std::string aregs[];
extern std::string regs8[];
extern std::string regs16[];
extern std::string sregs[];

inline uint16_t read16(uint8_t *mem) {
	return mem[0] | (mem[1] << 8);
}

inline uint32_t read32(uint8_t *mem) {
	return mem[0] | (mem[1] << 8) | (mem[2] << 16) | (mem[3] << 24);
}

inline uint16_t disp8(uint8_t *mem, uint16_t addr) {
	return addr + 1 + (int8_t)mem[0];
}

inline uint16_t disp16(uint8_t *mem, uint16_t addr) {
	return addr + 2 + (int16_t)read16(mem);
}

extern std::string hex(int v, int len = 0);
extern std::string readfar(uint8_t *mem);

enum OperandType { Reg, SReg, Imm, Ptr, ModRM };

struct Operand {
	int len;
	bool w;
	int type, value;
	Operand(int len, int type, int value);
	std::string str(bool w);
};

struct OpCode {
	bool prefix;
	size_t len;
	std::string mne, op1, op2;

	OpCode(
		int len = 0,
		const std::string &mne = "",
		const std::string &op1 = "",
		const std::string &op2 = "");
	OpCode(
		int len,
		const std::string &mne,
		uint16_t value,
		const std::string &op2 = "");
	OpCode(
		int len,
		const std::string &mne,
		const std::string &op1,
		uint16_t value);
	OpCode(const std::string &mne);

	inline bool empty() { return len == 0; }

	std::string str();
};

Operand modrm(uint8_t *mem, bool w);
OpCode disasm1(uint8_t *mem, uint16_t addr);
OpCode disasm1(uint8_t *mem, uint16_t addr, size_t last);
void disasm(uint8_t *mem, size_t size);

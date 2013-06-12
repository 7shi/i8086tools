#pragma once
#include "utils.h"
#include <vector>

extern const char *header;
extern bool verbose;
extern bool ptable[256];

class VM {
private:
	uint16_t ip, r[8];
	uint8_t *r8[8];
	uint8_t text[65536], mem[65536], *data;
	size_t tsize;
	bool OF, DF, SF, ZF, PF, CF;
	uint16_t start_sp;
public:
	int exit_status;

public:
	VM();
	bool read(const char *file);
	void run(const std::vector<std::string> &args);
	void disasm();

private:
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

	inline uint16_t read16(uint16_t addr) {
		return data[addr] | (data[addr + 1] << 8);
	}

	inline void write16(uint16_t addr, uint16_t value) {
		data[addr] = value;
		data[addr + 1] = value >> 8;
	}

	void debug(const OpCode &op);
	int addr(const Operand &opr);
	uint8_t get8(const Operand &opr);
	uint16_t get16(const Operand &opr);
	void set8(const Operand &opr, uint8_t value);
	void set16(const Operand &opr, uint16_t value);
	bool minix_syscall();
	bool run1(uint8_t prefix = 0);
};

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
#pragma once
#include "utils.h"
#include "OpCode.h"
#include <vector>
#include <list>

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

extern const char *header;
extern bool ptable[256];
extern int trace;

class VM {
private:
	static VM *current;
	uint16_t ip, r[8];
	uint8_t *r8[8];
	uint8_t *text, *data;
	size_t tsize, dsize;
	bool OF, DF, SF, ZF, PF, CF;
	uint16_t start_sp;
	bool hasExited;
	std::list<int> handles;
public:
	int exitcode;

public:
	VM();
	~VM();
	bool load(const std::string &fn);
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
		return ::read16(data + addr);
	}

	inline uint16_t read32(uint16_t addr) {
		return ::read32(data + addr);
	}

	inline void write16(uint16_t addr, uint16_t value) {
		::write16(data + addr, value);
	}

	void debug(const OpCode &op);
	int addr(const Operand &opr);
	uint8_t get8(const Operand &opr);
	uint16_t get16(const Operand &opr);
	void set8(const Operand &opr, uint8_t value);
	void set16(const Operand &opr, uint16_t value);
	void run1(uint8_t prefix = 0);

	struct syshandler {
		const char *name;
		void (VM::*f)();
	};
	static const int nsyscalls = 78;
	static syshandler syscalls[nsyscalls];

	void minix_syscall();
	void _exit     (); //  1
	void _read     (); //  3
	void _write    (); //  4
	void _open     (); //  5
	void _close    (); //  6
	void _unlink   (); // 10
	void _brk      (); // 17
	void _lseek    (); // 19
	void _signal   (); // 48
	void _sigaction(); // 71

	static void sighandler(int sig);
	struct sigact {
		uint16_t sa_handler;
		uint16_t sa_mask;
		int16_t sa_flags;
	};
	static const int nsig = 12;
	sigact sigacts[nsig];
};

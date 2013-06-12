#pragma once
#include "Operand.h"
#include <stdint.h>

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

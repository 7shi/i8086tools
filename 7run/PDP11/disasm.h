#pragma once
#include "OpCode.h"
#include "../VMBase.h"

namespace PDP11 {
    extern std::string regs[];

    OpCode disasm1(uint8_t *mem, uint16_t addr);
    OpCode disasm1p(uint8_t *mem, uint16_t addr, std::map<int, Symbol> *syms = NULL);
    void disasm(uint8_t *mem, size_t size, std::map<int, Symbol> *syms = NULL);
}

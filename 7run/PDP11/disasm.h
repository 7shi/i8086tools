#pragma once
#include "OpCode.h"
#include <map>

namespace PDP11 {
    extern std::string regs[];

    struct Symbol {
        std::string name;
        int type, addr;
    };

    OpCode disasm1(uint8_t *mem, uint16_t addr);
    void disasm(uint8_t *mem, size_t size, std::map<int, Symbol> *syms = NULL);
}

#pragma once
#include "OpCode.h"

namespace PDP11 {
    extern std::string regs[];

    OpCode disasm1(uint8_t *mem, uint16_t addr);
    void disasm(uint8_t *mem, size_t size);
}

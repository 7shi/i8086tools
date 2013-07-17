#pragma once
#include "OpCode.h"

namespace i8086 {
    extern std::string regs8[];
    extern std::string regs16[];
    extern std::string sregs[];

    OpCode disasm1(uint8_t *mem, uint16_t addr);
    OpCode disasm1(uint8_t *mem, uint16_t addr, size_t size);
    OpCode disasm1p(uint8_t *mem, uint16_t addr, size_t size);
    void disasm(uint8_t *mem, size_t size);
}

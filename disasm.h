#pragma once
#include "OpCode.h"

Operand modrm(uint8_t *mem, bool w);
OpCode disasm1(uint8_t *mem, uint16_t addr);
OpCode disasm1(uint8_t *mem, uint16_t addr, size_t last);
void disasm(uint8_t *mem, size_t size);

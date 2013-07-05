#include "disasm.h"
#include <stdio.h>

using namespace PDP11;

std::string PDP11::regs[] = {"r0", "r1", "r2", "r3", "r4", "r5", "sp", "pc"};

static int undefined;

static OpCode srcdst(uint8_t *mem, uint16_t addr, int w, const char *mne) {
    Operand opr1(mem + 2, addr + 2, w >> 6);
    int offset = 2 + opr1.len;
    Operand opr2(mem + offset, addr + offset, w);
    return OpCode(offset + opr2.len, mne, opr1, opr2);
}

OpCode PDP11::disasm1(uint8_t *mem, uint16_t addr) {
    uint16_t w = ::read16(mem);
    switch (w >> 12) {
        case 000:
            break;
        case 001: return srcdst(mem, addr, w, "mov");
        case 002: return srcdst(mem, addr, w, "cmp");
        case 003: return srcdst(mem, addr, w, "bit");
        case 004: return srcdst(mem, addr, w, "bic");
        case 005: return srcdst(mem, addr, w, "bis");
        case 006: return srcdst(mem, addr, w, "add");
        case 007:
            break;
        case 010:
            break;
        case 011: return srcdst(mem, addr, w, "movb");
        case 012: return srcdst(mem, addr, w, "cmpb");
        case 013: return srcdst(mem, addr, w, "bitb");
        case 014: return srcdst(mem, addr, w, "bicb");
        case 015: return srcdst(mem, addr, w, "bisb");
        case 016: return srcdst(mem, addr, w, "sub");
        case 017:
            break;
    }
    undefined++;
    return OpCode(2, "(undefined)");
}

void PDP11::disasm(uint8_t *mem, size_t size) {
    undefined = 0;
    int index = 0;
    while (index < (int) size) {
        OpCode op = disasm1(mem + index, index);
        std::string ops = op.str();
        for (int i = 0; i < (int)op.len; i += 6) {
            int len = op.len - i;
            if (len > 6) len = 6;
            std::string hex = hexdump2(mem + index + i, len);
            if (i == 0) {
                printf("%04x: %-14s %s\n", index, hex.c_str(), ops.c_str());
            } else {
                printf("      %-14s\n", hex.c_str());
            }
        }
        index += op.len;
    }
    if (undefined) printf("undefined: %d\n", undefined);
}

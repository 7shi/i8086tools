#include "VM.h"
#include "disasm.h"
#include "regs.h"
#include <stdio.h>

using namespace PDP11;

void VM::run1() {
    OpCode *op, op1;
    if (cache.empty()) {
        op = &(op1 = disasm1(text + PC, PC));
    } else {
        if (cache[PC].len > 0) {
            op = &cache[PC];
        } else {
            op = &(cache[PC] = disasm1(text + PC, PC));
        }
    }
    if (PC + op->len > tsize) {
        fprintf(stderr, "overrun: %04x\n", PC);
        hasExited = true;
        return;
    }
    if (SP < brksize) {
        fprintf(stderr, "stack overflow: %04x\n", SP);
        hasExited = true;
        return;
    }
    if (trace >= 2) debug(PC, *op);
    uint16_t w = ::read16(text + PC);
    uint16_t oldpc = PC;
    int dst, src, val;
    uint16_t val16;
    uint8_t val8;
    PC += op->len;
    switch (w >> 12) {
        case 000:
            break;
        case 001: // mov: MOVe
            src = get16(op->opr1);
            set16(op->opr2, src);
            setZNCV(src == 0, int16_t(uint16_t(src)) < 0, C, false);
            return;
        case 002: // cmp: CoMPare
            src = get16(op->opr1);
            dst = get16(op->opr2);
            val16 = val = int16_t(uint16_t(src)) - int16_t(uint16_t(dst));
            setZNCV(val16 == 0, val16 < 0, src < dst, val != val16);
            return;
        case 003: // bit: BIt Test
            val = get16(op->opr1) & get16(op->opr2);
            setZNCV(val == 0, (val & 0x8000) != 0, C, false);
            return;
        case 004: // bic: BIt Clear
            val = (~get16(op->opr1)) & get16(op->opr2, true);
            set16(op->opr2, val);
            setZNCV(val == 0, (val & 0x8000) != 0, C, false);
            return;
        case 005: // bis: BIt Set
            val = get16(op->opr1) | get16(op->opr2, true);
            set16(op->opr2, val);
            setZNCV(val == 0, (val & 0x8000) != 0, C, false);
            return;
        case 006: // add: ADD
            src = get16(op->opr1);
            dst = get16(op->opr2, true);
            val16 = val = int16_t(uint16_t(src)) + int16_t(uint16_t(dst));
            set16(op->opr2, val16);
            setZNCV(val16 == 0, val16 < 0, src + dst >= 0x10000, val != val16);
            return;
        case 007:
            break;
        case 010:
            break;
        case 011: // movb: MOVe Byte
            src = get8(op->opr1);
            set8(op->opr2, src);
            setZNCV(src == 0, int8_t(uint8_t(src)) < 0, C, false);
            return;
        case 012: // cmpb: CoMPare Byte
            src = get8(op->opr1);
            dst = get8(op->opr2);
            val8 = val = int8_t(uint8_t(src)) - int8_t(uint8_t(dst));
            setZNCV(val8 == 0, val8 < 0, src < dst, val != val8);
            return;
        case 013: // bitb: BIt Test Byte
            val = get8(op->opr1) & get8(op->opr2);
            setZNCV(val == 0, (val & 0x80) != 0, C, false);
            return;
        case 014: // bicb: BIt Clear Byte
            val = (~get8(op->opr1)) & get8(op->opr2, true);
            set8(op->opr2, val);
            setZNCV(val == 0, (val & 0x80) != 0, C, false);
            return;
        case 015: // bisb: BIt Set Byte
            val = get8(op->opr1) | get8(op->opr2, true);
            set8(op->opr2, val);
            setZNCV(val == 0, (val & 0x80) != 0, C, false);
            return;
        case 016: // sub: SUBtract
            src = get16(op->opr1);
            dst = get16(op->opr2);
            val16 = val = int16_t(uint16_t(dst)) - int16_t(uint16_t(src));
            set16(op->opr2, val16);
            setZNCV(val16 == 0, val16 < 0, dst < src, val != val16);
            return;
        case 017:
            break;
    }
    if (trace < 2) {
        fprintf(stderr, header);
        debug(oldpc, *op);
    }
    fprintf(stderr, "not implemented\n");
    hasExited = true;
}

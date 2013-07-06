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
    int opr1 = op->opr1.value, opr2 = op->opr2.value;
    if (trace >= 2) debug(PC, *op);
    uint8_t w = ::read16(text + PC);
    uint16_t oldpc = PC;
    PC += op->len;
    switch (w) {
    }
    if (trace < 2) {
        fprintf(stderr, header);
        debug(oldpc, *op);
    }
    fprintf(stderr, "not implemented\n");
    hasExited = true;
}

#include "Operand.h"
#include "disasm.h"

using namespace PDP11;

Operand::Operand()
: len(-1), mode(0), reg(0), value(0) {
}

Operand::Operand(int len, int mode, int reg, int value)
: len(len), mode(mode), reg(reg), value(value) {
}

std::string Operand::str() const {
    if (reg == 7) {
        switch (mode) {
            case 2: return "$" + hex(value);
            case 3: return "*$" + hex(value);
            case 6: return hex(value);
            case 7: return "*" + hex(value);
        }
    }
    const std::string &rn = regs[reg];
    switch (mode) {
        case 0: return rn;
        case 1: return "(" + rn + ")";
        case 2: return "(" + rn + ")+";
        case 3: return "*(" + rn + ")+";
        case 4: return "-(" + rn + ")";
        case 5: return "*-(" + rn + ")";
        case 6: return hex(value) + "(" + rn + ")";
        case 7: return "*" + hex(value) + "(" + rn + ")";
    }
    return "?";
}

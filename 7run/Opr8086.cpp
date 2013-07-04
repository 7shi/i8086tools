#include "Opr8086.h"

namespace i8086 {
    Opr8086 dx = Opr8086(0, true, Reg, 2);
    Opr8086 cl = Opr8086(0, false, Reg, 1);
    Opr8086 es = Opr8086(0, true, SReg, 0);
    Opr8086 cs = Opr8086(0, true, SReg, 1);
    Opr8086 ss = Opr8086(0, true, SReg, 2);
    Opr8086 ds = Opr8086(0, true, SReg, 3);
}

using namespace i8086;

Opr8086::Opr8086()
: len(-1), w(false), type(0), value(0), seg(-1) {
}

Opr8086::Opr8086(int len, bool w, int type, int value)
: len(len), w(w), type(type), value(value), seg(-1) {
}

std::string Opr8086::str() const {
    switch (type) {
        case Reg: return (w ? regs16: regs8)[value];
        case SReg: return sregs[value];
        case Imm: return hex(value, len == 2 ? 4: 0);
        case Addr: return hex(value, 4);
        case Far: return hex((value >> 16) & 0xffff, 4) + ":" + hex(value & 0xffff, 4);
    }
    std::string ret = "[";
    if (seg >= 0) ret += sregs[seg] + ":";
    if (type == Ptr) return ret + hex(value, 4) + "]";
    const char *ms[] = {"bx+si", "bx+di", "bp+si", "bp+di", "si", "di", "bp", "bx"};
    ret += ms[type - ModRM];
    if (value > 0) {
        ret += "+" + hex(value);
    } else if (value < 0) {
        ret += hex(value);
    }
    return ret + "]";
}

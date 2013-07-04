#pragma once
#include "utils.h"
#include <string>

namespace i8086 {

    enum OprType8086 {
        Reg, SReg, Imm, Addr, Far, Ptr, ModRM
    };

    struct Opr8086 {
        int len;
        bool w;
        int type, value, seg;

        Opr8086();
        Opr8086(int len, bool w, int type, int value);

        inline bool empty() const {
            return len == -1;
        }

        std::string str() const;
    };

    extern Opr8086 dx, cl, es, cs, ss, ds;

    inline Opr8086 reg(int r, bool w) {
        return Opr8086(0, w, Reg, r);
    }

    inline Opr8086 imm16(uint16_t v) {
        return Opr8086(2, true, Imm, v);
    }

    inline Opr8086 imm8(uint8_t v) {
        return Opr8086(1, false, Imm, v);
    }

    inline Opr8086 far(uint32_t a) {
        return Opr8086(4, false, Far, a);
    }

    inline Opr8086 ptr(uint16_t a, bool w) {
        return Opr8086(2, w, Ptr, a);
    }

    inline Opr8086 disp8(uint8_t *mem, uint16_t addr) {
        return Opr8086(1, false, Addr, (uint16_t) (addr + 1 + (int8_t) mem[0]));
    }

    inline Opr8086 disp16(uint8_t *mem, uint16_t addr) {
        return Opr8086(2, true, Addr, (uint16_t) (addr + 2 + (int16_t) read16(mem)));
    }
}

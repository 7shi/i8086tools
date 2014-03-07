#pragma once
#include "../utils.h"
#include <string>

namespace i8086 {

    enum OperandType {
        Reg, SReg, Imm, Addr, Far, Ptr, ModRM
    };

    struct VM;

    struct Operand {
        VM *vm;
        int len;
        bool w;
        int type, value, seg;

        inline bool empty() const {
            return len < 0;
        }

        std::string str() const;
    };

    inline Operand getopr(int len, bool w, int type, int value, int seg = -1) {
        Operand ret = {NULL, len, w, type, value, seg};
        return ret;
    }

    extern Operand noopr, dx, cl, es, cs, ss, ds;
}

#pragma once
#include "../utils.h"
#include <string>

namespace PDP11 {

    struct Operand {
        int len, mode, reg, value;

        Operand();
        Operand(int len, int mode, int reg, int value = 0);
        Operand(uint8_t *mem, int pc, int modr);

        inline bool empty() const {
            return len == -1;
        }

        std::string str() const;
    };

    inline Operand reg(int r) {
        return Operand(0, 0, r & 7);
    }
}

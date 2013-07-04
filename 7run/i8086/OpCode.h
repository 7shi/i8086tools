#pragma once
#include "Operand.h"
#include <stdint.h>

namespace i8086 {

    struct OpCode {
        const char *prefix;
        size_t len;
        const char *mne;
        Operand opr1, opr2;

        OpCode();
        OpCode(int len, const char *mne,
                const Operand &opr1 = Operand(),
                const Operand &opr2 = Operand());
        OpCode(const char *mne, const Operand &opr = Operand());

        inline bool empty() const {
            return len == 0;
        }
        std::string str() const;
        void swap();
    };
}

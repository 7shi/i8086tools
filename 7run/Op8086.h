#pragma once
#include "Opr8086.h"
#include <stdint.h>

struct Op8086 {
    bool prefix;
    size_t len;
    std::string mne;
    Opr8086 opr1, opr2;

    Op8086();
    Op8086(
            int len, const std::string &mne,
            const Opr8086 &opr1 = Opr8086(),
            const Opr8086 &opr2 = Opr8086());
    Op8086(const std::string &mne, const Opr8086 &opr = Opr8086());

    inline bool empty() const {
        return len == 0;
    }
    std::string str() const;
    void swap();
};

#include "OpCode.h"

using namespace PDP11;

OpCode::OpCode()
: len(0) {
}

OpCode::OpCode(
        int len, const char *mne, const Operand &opr1, const Operand &opr2)
: len(len), mne(mne), opr1(opr1), opr2(opr2) {
}

std::string OpCode::str() const {
    std::string mne = this->mne;
    if (opr1.empty()) return mne;
    if (opr2.empty()) return mne + " " + opr1.str();
    return mne + " " + opr1.str() + ", " + opr2.str();
}
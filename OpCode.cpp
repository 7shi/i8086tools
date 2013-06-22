#include "OpCode.h"

OpCode::OpCode()
: prefix(false), len(0) {
}

OpCode::OpCode(
        int len, const std::string &mne, const Operand &opr1, const Operand &opr2)
: prefix(false), len(len), mne(mne), opr1(opr1), opr2(opr2) {
}

OpCode::OpCode(const std::string &mne, const Operand &opr)
: prefix(true), len(1), mne(mne), opr2(opr) {
}

std::string OpCode::str() const {
    if (opr1.empty()) return mne;
    std::string opr1s = opr1.str();
    if (opr1.type >= Ptr && !opr1.w && (opr2.empty() || opr2.type == Imm))
        opr1s = "byte " + opr1s;
    if (opr2.empty()) return mne + " " + opr1s;
    return mne + " " + opr1s + ", " + opr2.str();
}

void OpCode::swap() {
    Operand tmp = opr1;
    opr1 = opr2;
    opr2 = tmp;
}

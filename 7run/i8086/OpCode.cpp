#include "OpCode.h"

using namespace i8086;

OpCode::OpCode()
: prefix(NULL), len(0) {
}

OpCode::OpCode(
        int len, const char *mne, const Operand &opr1, const Operand &opr2)
: prefix(NULL), len(len), mne(mne), opr1(opr1), opr2(opr2) {
}

OpCode::OpCode(const char *mne, const Operand &opr)
: prefix(mne), len(1), mne(mne), opr2(opr) {
}

std::string OpCode::str() const {
    std::string mne = this->mne;
    if (prefix) {
        std::string pfx = prefix;
        if (mne != "cmps" && mne != "scas" && startsWith(pfx, "rep")) {
            mne = "rep " + mne;
        } else {
            mne = pfx + " " + mne;
        }
    }
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

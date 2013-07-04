#include "Op8086.h"

using namespace i8086;

Op8086::Op8086()
: prefix(false), len(0) {
}

Op8086::Op8086(
        int len, const std::string &mne, const Opr8086 &opr1, const Opr8086 &opr2)
: prefix(false), len(len), mne(mne), opr1(opr1), opr2(opr2) {
}

Op8086::Op8086(const std::string &mne, const Opr8086 &opr)
: prefix(true), len(1), mne(mne), opr2(opr) {
}

std::string Op8086::str() const {
    if (opr1.empty()) return mne;
    std::string opr1s = opr1.str();
    if (opr1.type >= Ptr && !opr1.w && (opr2.empty() || opr2.type == Imm))
        opr1s = "byte " + opr1s;
    if (opr2.empty()) return mne + " " + opr1s;
    return mne + " " + opr1s + ", " + opr2.str();
}

void Op8086::swap() {
    Opr8086 tmp = opr1;
    opr1 = opr2;
    opr2 = tmp;
}

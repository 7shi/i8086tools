#include "disasm.h"
#include <stdio.h>

using namespace PDP11;

std::string PDP11::regs[] = {"r0", "r1", "r2", "r3", "r4", "r5", "sp", "pc"};

static int undefined;

static inline OpCode srcdst(uint8_t *mem, uint16_t addr, int w, const char *mne) {
    Operand opr1(mem + 2, addr + 2, w >> 6);
    int offset = 2 + opr1.len;
    Operand opr2(mem + offset, addr + offset, w);
    return OpCode(offset + opr2.len, mne, opr1, opr2);
}

static inline OpCode dst(uint8_t *mem, uint16_t addr, int w, const char *mne) {
    Operand opr(mem + 2, addr + 2, w);
    return OpCode(2 + opr.len, mne, opr);
}

OpCode PDP11::disasm1(uint8_t *mem, uint16_t addr) {
    uint16_t w = ::read16(mem);
    switch (w >> 12) {
        case 000:
            switch ((w >> 6) & 077) {
                case 000:
                    switch (w & 7) {
                        case 0: return OpCode(1, "halt");
                        case 1: return OpCode(1, "wait");
                        case 2: return OpCode(1, "rti");
                        case 3: return OpCode(1, "bpt");
                        case 4: return OpCode(1, "iot");
                        case 5: return OpCode(1, "reset");
                        case 6: return OpCode(1, "rtt");
                    }
                    break;
                case 001: return dst(mem, addr, w, "jmp");
                case 002: break;
                case 003: return dst(mem, addr, w, "swab");
                case 004:
                case 005:
                case 006:
                case 007:
                case 010:
                case 011:
                case 012:
                case 013:
                case 014:
                case 015:
                case 016:
                case 017:
                case 020:
                case 021:
                case 022:
                case 023:
                case 024:
                case 025:
                case 026:
                case 027:
                case 030:
                case 031:
                case 032:
                case 033:
                case 034:
                case 035:
                case 036:
                case 037:
                case 040:
                case 041:
                case 042:
                case 043:
                case 044:
                case 045:
                case 046:
                case 047: break;
                case 050: return dst(mem, addr, w, "clr");
                case 051: return dst(mem, addr, w, "com");
                case 052: return dst(mem, addr, w, "inc");
                case 053: return dst(mem, addr, w, "dec");
                case 054: return dst(mem, addr, w, "neg");
                case 055: return dst(mem, addr, w, "adc");
                case 056: return dst(mem, addr, w, "sbc");
                case 057: return dst(mem, addr, w, "tst");
                case 060: return dst(mem, addr, w, "ror");
                case 061: return dst(mem, addr, w, "rol");
                case 062: return dst(mem, addr, w, "asr");
                case 063: return dst(mem, addr, w, "asl");
                case 064: break;
                case 065: return dst(mem, addr, w, "mfpi");
                case 066: return dst(mem, addr, w, "mtpi");
                case 067: return dst(mem, addr, w, "sxt");
            }
            break;
        case 001: return srcdst(mem, addr, w, "mov");
        case 002: return srcdst(mem, addr, w, "cmp");
        case 003: return srcdst(mem, addr, w, "bit");
        case 004: return srcdst(mem, addr, w, "bic");
        case 005: return srcdst(mem, addr, w, "bis");
        case 006: return srcdst(mem, addr, w, "add");
        case 007:
            break;
        case 010:
            switch ((w >> 6) & 077) {
                case 050: return dst(mem, addr, w, "clrb");
                case 051: return dst(mem, addr, w, "comb");
                case 052: return dst(mem, addr, w, "incb");
                case 053: return dst(mem, addr, w, "decb");
                case 054: return dst(mem, addr, w, "negb");
                case 055: return dst(mem, addr, w, "adcb");
                case 056: return dst(mem, addr, w, "sbcb");
                case 057: return dst(mem, addr, w, "tstb");
                case 060: return dst(mem, addr, w, "rorb");
                case 061: return dst(mem, addr, w, "rolb");
                case 062: return dst(mem, addr, w, "asrb");
                case 063: return dst(mem, addr, w, "aslb");
                case 064: break;
                case 065: return dst(mem, addr, w, "mfpd");
                case 066: return dst(mem, addr, w, "mtpd");
            }
            break;
        case 011: return srcdst(mem, addr, w, "movb");
        case 012: return srcdst(mem, addr, w, "cmpb");
        case 013: return srcdst(mem, addr, w, "bitb");
        case 014: return srcdst(mem, addr, w, "bicb");
        case 015: return srcdst(mem, addr, w, "bisb");
        case 016: return srcdst(mem, addr, w, "sub");
        case 017:
            break;
    }
    undefined++;
    return OpCode(2, "(undefined)");
}

void PDP11::disasm(uint8_t *mem, size_t size) {
    undefined = 0;
    int index = 0;
    while (index < (int) size) {
        OpCode op = disasm1(mem + index, index);
        std::string ops = op.str();
        for (int i = 0; i < (int) op.len; i += 6) {
            int len = op.len - i;
            if (len > 6) len = 6;
            std::string hex = hexdump2(mem + index + i, len);
            if (i == 0) {
                printf("%04x: %-14s %s\n", index, hex.c_str(), ops.c_str());
            } else {
                printf("      %-14s\n", hex.c_str());
            }
        }
        index += op.len;
    }
    if (undefined) printf("undefined: %d\n", undefined);
}

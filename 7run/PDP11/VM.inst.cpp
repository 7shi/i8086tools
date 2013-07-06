#include "VM.h"
#include "disasm.h"
#include "regs.h"
#include <stdio.h>

using namespace PDP11;

void VM::run1() {
    OpCode *op, op1;
    if (cache.empty()) {
        op = &(op1 = disasm1(text + PC, PC));
    } else {
        if (cache[PC].len > 0) {
            op = &cache[PC];
        } else {
            op = &(cache[PC] = disasm1(text + PC, PC));
        }
    }
    if (PC + op->len > tsize) {
        fprintf(stderr, "overrun: %04x\n", PC);
        hasExited = true;
        return;
    }
    if (SP < brksize) {
        fprintf(stderr, "stack overflow: %04x\n", SP);
        hasExited = true;
        return;
    }
    if (trace >= 2) debug(PC, *op);
    uint16_t w = ::read16(text + PC);
    uint16_t oldpc = PC;
    int dst, src, val;
    uint16_t val16;
    uint8_t val8;
    PC += op->len;
    switch (w >> 12) {
        case 000:
            switch ((w >> 6) & 077) {
                case 000:
                    switch (w & 7) {
                        case 0: // halt
                        case 1: // wait
                        case 2: // rti
                        case 3: // bpt
                        case 4: // iot
                        case 5: // reset
                        case 6: // rtt
                            break;
                    }
                    break;
                case 001: // jmp
                case 002:
                    switch ((w >> 3) & 7) {
                        case 0: // rts
                        case 3: // spl
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                            switch (w) {
                                case 0240: // nop
                                case 0241: // clc
                                case 0242: // clv
                                case 0243: // clvc
                                case 0244: // clz
                                case 0245: // clzc
                                case 0246: // clzv
                                case 0247: // clzvc
                                case 0250: // cln
                                case 0251: // clnc
                                case 0252: // clnv
                                case 0253: // clnvc
                                case 0254: // clnz
                                case 0255: // clnzc
                                case 0256: // clnzv
                                case 0257: // ccc
                                case 0261: // sec
                                case 0262: // sev
                                case 0263: // sevc
                                case 0264: // sez
                                case 0265: // sezc
                                case 0266: // sezv
                                case 0267: // sezvc
                                case 0270: // sen
                                case 0271: // senc
                                case 0272: // senv
                                case 0273: // senvc
                                case 0274: // senz
                                case 0275: // senzc
                                case 0276: // senzv
                                case 0277: // scc
                                    break;
                            }
                    }
                    break;
                case 003: // swab
                case 004:
                case 005:
                case 006:
                case 007: // br
                case 010:
                case 011:
                case 012:
                case 013: // bne
                case 014:
                case 015:
                case 016:
                case 017: // beq
                case 020:
                case 021:
                case 022:
                case 023: // bge
                case 024:
                case 025:
                case 026:
                case 027: // blt
                case 030:
                case 031:
                case 032:
                case 033: // bgt
                case 034:
                case 035:
                case 036:
                case 037: // ble
                case 040:
                case 041:
                case 042:
                case 043:
                case 044:
                case 045:
                case 046:
                case 047: // jsr
                case 050: // clr
                case 051: // com
                case 052: // inc
                case 053: // dec
                case 054: // neg
                case 055: // adc
                case 056: // sbc
                case 057: // tst
                case 060: // ror
                case 061: // rol
                case 062: // asr
                case 063: // asl
                case 064: // mark
                case 065: // mfpi
                case 066: // mtpi
                case 067: // sxt
                    break;
            }
            break;
        case 001: // mov: MOVe
            src = get16(op->opr1);
            set16(op->opr2, src);
            setZNCV(src == 0, int16_t(uint16_t(src)) < 0, C, false);
            return;
        case 002: // cmp: CoMPare
            src = get16(op->opr1);
            dst = get16(op->opr2);
            val16 = val = int16_t(uint16_t(src)) - int16_t(uint16_t(dst));
            setZNCV(val16 == 0, val16 < 0, src < dst, val != val16);
            return;
        case 003: // bit: BIt Test
            val = get16(op->opr1) & get16(op->opr2);
            setZNCV(val == 0, (val & 0x8000) != 0, C, false);
            return;
        case 004: // bic: BIt Clear
            val = (~get16(op->opr1)) & get16(op->opr2, true);
            set16(op->opr2, val);
            setZNCV(val == 0, (val & 0x8000) != 0, C, false);
            return;
        case 005: // bis: BIt Set
            val = get16(op->opr1) | get16(op->opr2, true);
            set16(op->opr2, val);
            setZNCV(val == 0, (val & 0x8000) != 0, C, false);
            return;
        case 006: // add: ADD
            src = get16(op->opr1);
            dst = get16(op->opr2, true);
            val16 = val = int16_t(uint16_t(src)) + int16_t(uint16_t(dst));
            set16(op->opr2, val16);
            setZNCV(val16 == 0, val16 < 0, src + dst >= 0x10000, val != val16);
            return;
        case 007:
            switch ((w >> 9) & 7) {
                case 0: // mul
                case 1: // div
                case 2: // ash
                case 3: // ashc
                case 4: // xor
                case 7: // sob
                    break;
            }
            break;
        case 010:
            switch ((w >> 6) & 077) {
                case 000:
                case 001:
                case 002:
                case 003: // bpl
                case 004:
                case 005:
                case 006:
                case 007: // bmi
                case 010:
                case 011:
                case 012:
                case 013: // bhi
                case 014:
                case 015:
                case 016:
                case 017: // blos
                case 020:
                case 021:
                case 022:
                case 023: // bvc
                case 024:
                case 025:
                case 026:
                case 027: // bvs
                case 030:
                case 031:
                case 032:
                case 033: // bcc
                case 034:
                case 035:
                case 036:
                case 037: // bcs
                case 040:
                case 041:
                case 042:
                case 043: // emt
                case 044:
                case 045:
                case 046:
                case 047: // sys
                case 050: // clrb
                case 051: // comb
                case 052: // incb
                case 053: // decb
                case 054: // negb
                case 055: // adcb
                case 056: // sbcb
                case 057: // tstb
                case 060: // rorb
                case 061: // rolb
                case 062: // asrb
                case 063: // aslb
                case 064: break;
                case 065: // mfpd
                case 066: // mtpd
                    break;
            }
            break;
        case 011: // movb: MOVe Byte
            src = get8(op->opr1);
            set8(op->opr2, src);
            setZNCV(src == 0, int8_t(uint8_t(src)) < 0, C, false);
            return;
        case 012: // cmpb: CoMPare Byte
            src = get8(op->opr1);
            dst = get8(op->opr2);
            val8 = val = int8_t(uint8_t(src)) - int8_t(uint8_t(dst));
            setZNCV(val8 == 0, val8 < 0, src < dst, val != val8);
            return;
        case 013: // bitb: BIt Test Byte
            val = get8(op->opr1) & get8(op->opr2);
            setZNCV(val == 0, (val & 0x80) != 0, C, false);
            return;
        case 014: // bicb: BIt Clear Byte
            val = (~get8(op->opr1)) & get8(op->opr2, true);
            set8(op->opr2, val);
            setZNCV(val == 0, (val & 0x80) != 0, C, false);
            return;
        case 015: // bisb: BIt Set Byte
            val = get8(op->opr1) | get8(op->opr2, true);
            set8(op->opr2, val);
            setZNCV(val == 0, (val & 0x80) != 0, C, false);
            return;
        case 016: // sub: SUBtract
            src = get16(op->opr1);
            dst = get16(op->opr2);
            val16 = val = int16_t(uint16_t(dst)) - int16_t(uint16_t(src));
            set16(op->opr2, val16);
            setZNCV(val16 == 0, val16 < 0, dst < src, val != val16);
            return;
        case 017:
            switch (w) {
                case 0170011: return; // setd: SET Double
            }
            break;
    }
    if (trace < 2) {
        fprintf(stderr, header);
        debug(oldpc, *op);
    }
    fprintf(stderr, "not implemented\n");
    hasExited = true;
}

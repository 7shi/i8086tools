#include "VM.h"
#include "disasm.h"
#include "regs.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

using namespace PDP11;

const char *PDP11::header = " r0   r1   r2   r3   r4   r5   sp  flags pc\n";

void VM::showHeader() {
    fprintf(stderr, header);
}

void VM::debug(uint16_t ip, const OpCode &op) {
    fprintf(stderr,
            "%04x %04x %04x %04x %04x %04x %04x %c%c%c%c %04x:%-14s %s",
            r[0], r[1], r[2], r[3], r[4], r[5], r[6],
            "-Z"[Z], "-N"[N], "-C"[C], "-V"[V],
            PC, hexdump2(text + ip, op.len).c_str(), op.str().c_str());
    if (trace >= 3) {
        uint16_t r[8];
        memcpy(r, this->r, sizeof (r));
        int ad1 = addr(op.opr1);
        int ad2 = addr(op.opr2);
        if (ad1 >= 0) {
            if (op.opr1.w)
                fprintf(stderr, " ;[%04x]%04x", ad1, read16(ad1));
            else
                fprintf(stderr, " ;[%04x]%02x", ad1, read8(ad1));
        }
        if (ad2 >= 0) {
            if (op.opr2.w)
                fprintf(stderr, " ;[%04x]%04x", ad2, read16(ad2));
            else
                fprintf(stderr, " ;[%04x]%02x", ad2, read8(ad2));
        }
        memcpy(this->r, r, sizeof (r));
    }
    fprintf(stderr, "\n");
}

VM::VM() : start_sp(0) {
    memset(r, 0, sizeof (r));
    Z = N = C = V = false;
}

VM::~VM() {
}

int VM::addr(const Operand &opr, bool nomove) {
    if (opr.reg == 7) {
        switch (opr.mode) {
            case 1:
            case 3:
            case 6: return opr.value;
            case 7: return read16(opr.value);
        }
        return -1;
    }
    if (nomove) {
        switch (opr.mode) {
            case 1: return r[opr.reg];
            case 2: return r[opr.reg];
            case 3: return read16(r[opr.reg]);
            case 4: return r[opr.reg] - opr.diff();
            case 5: return read16(r[opr.reg] - opr.diff());
            case 6: return r[opr.reg] + opr.value;
            case 7: return read16(r[opr.reg] + opr.value);
        }
    }
    switch (opr.mode) {
        case 1: return r[opr.reg];
        case 2: return getInc(opr);
        case 3: return read16(getInc(opr));
        case 4: return getDec(opr);
        case 5: return read16(getDec(opr));
        case 6: return r[opr.reg] + opr.value;
        case 7: return read16(r[opr.reg] + opr.value);
    }
    return -1;
}

uint8_t VM::get8(const Operand &opr, bool nomove) {
    if (opr.mode == 0) return r[opr.reg];
    int ad = addr(opr, nomove);
    return ad < 0 ? opr.value : read8(ad);
}

uint16_t VM::get16(const Operand &opr, bool nomove) {
    if (opr.mode == 0) return r[opr.reg];
    int ad = addr(opr, nomove);
    return ad < 0 ? opr.value : read16(ad);
}

void VM::set8(const Operand &opr, uint8_t value) {
    if (opr.mode == 0) {
        r[opr.reg] = (int16_t) (int8_t) value;
    } else {
        int ad = addr(opr);
        if (ad >= 0) write8(ad, value);
    }
}

void VM::set16(const Operand &opr, uint16_t value) {
    if (opr.mode == 0) {
        r[opr.reg] = value;
    } else {
        int ad = addr(opr);
        if (ad >= 0) write16(ad, value);
    }
}

bool VM::load2(const std::string &fn, FILE *f) {
    if (tsize > 0xffff) {
        fprintf(stderr, "too long raw binary: %s\n", fn.c_str());
        return false;
    }
    PC = 0;
    cache.clear();
    data = text;
    fread(text, 1, tsize, f);
    brksize = dsize = tsize;
    return true;
}

void VM::setArgs(
        const std::vector<std::string> &args,
        const std::vector<std::string> &envs) {
    int slen = 0;
    for (int i = 0; i < (int) args.size(); i++) {
        slen += args[i].size() + 1;
    }
    SP -= (slen + 1) & ~1;
    uint16_t ad1 = SP;
    SP -= (1 + args.size()) * 2;
    uint16_t ad2 = start_sp = SP;
    write16(SP, args.size()); // argc
    for (int i = 0; i < (int) args.size(); i++) {
        write16(ad2 += 2, ad1);
        strcpy((char *) data + ad1, args[i].c_str());
        ad1 += args[i].size() + 1;
    }
}

void VM::run2() {
    while (!hasExited) run1();
}

void VM::disasm() {
    ::disasm(text, tsize);
}

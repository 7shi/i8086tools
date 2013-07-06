#include "VM.h"
#include "disasm.h"
#include "regs.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

using namespace PDP11;

bool PDP11::check(uint8_t h[2]) {
    int magic = ::read16(h);
    return magic == 0407 || magic == 0410 || magic == 0411;
}

const char *PDP11::header = " r0   r1   r2   r3   r4   r5   sp  flags pc\n";

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

uint16_t VM::getInc(const Operand &opr) {
    uint16_t ret = r[opr.reg];
    r[opr.reg] += opr.diff();
    return ret;
}

uint16_t VM::getDec(const Operand &opr) {
    r[opr.reg] -= opr.diff();
    return r[opr.reg];
}

int VM::addr(const Operand &opr, bool nomove) {
    if (opr.reg == 7) {
        switch (opr.mode) {
            case 3: return opr.value;
            case 6: return opr.value;
            case 7: return read16(opr.value);
        }
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

bool VM::load(const std::string &fn) {
    std::string fn2 = convpath(fn);
    const char *file = fn2.c_str();
    struct stat st;
    if (stat(file, &st)) {
        fprintf(stderr, "can not stat: %s\n", file);
        return false;
    }
    tsize = st.st_size;
    FILE *f = fopen(file, "rb");
    if (!f) {
        fprintf(stderr, "can not open: %s\n", file);
        return false;
    }
    if (tsize >= 0x10) {
        uint8_t h[0x10];
        if (fread(h, sizeof (h), 1, f) && check(h)) {
            tsize = ::read16(h + 2);
            dsize = ::read16(h + 4);
            PC = ::read16(h + 10);
            cache.clear();
            if (h[0] == 9) { // 0411
                cache.resize(0x10000);
                data = new uint8_t[0x10000];
                memset(data, 0, 0x10000);
                fread(text, 1, tsize, f);
                fread(data, 1, dsize, f);
            } else {
                data = text;
                if (h[0] == 8) { // 0410
                    cache.resize(0x10000);
                    fread(text, 1, tsize, f);
                    uint16_t doff = (tsize + 0x1fff) & ~0x1fff;
                    fread(text + doff, 1, dsize, f);
                    dsize += doff;
                } else {
                    dsize += tsize;
                    fread(text, 1, dsize, f);
                }
            }
            dsize += ::read16(h + 6); // bss
            brksize = dsize;
        } else {
            fseek(f, 0, SEEK_SET);
        }
    }
    if (!data) {
        if (tsize > 0xffff) {
            fprintf(stderr, "too long raw binary: %s\n", file);
            fclose(f);
            return false;
        }
        cache.clear();
        data = text;
        fread(text, 1, tsize, f);
        brksize = dsize = tsize;
    }
    fclose(f);
    return true;
}

void VM::run(
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
    run();
}

void VM::run() {
    VMUnix *from = current;
    //swtch(this);
    current = this;
    hasExited = false;
    while (!hasExited) run1();
    //swtch(from);
    current = from;
}

void VM::disasm() {
    ::disasm(text, tsize);
}

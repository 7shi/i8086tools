#include "VM.h"
#include "disasm.h"
#include "regs.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

using namespace i8086;

const char *i8086::header = " AX   BX   CX   DX   SP   BP   SI   DI  FLAGS IP\n";

bool VM::ptable[256];

void VM::debug(uint16_t ip, const OpCode &op) {
    fprintf(stderr,
            "%04x %04x %04x %04x %04x %04x %04x %04x %c%c%c%c %04x:%-12s %s",
            r[0], r[3], r[1], r[2], r[4], r[5], r[6], r[7],
            "-O"[OF], "-S"[SF], "-Z"[ZF], "-C"[CF],
            ip, hexdump(text + ip, op.len).c_str(), op.str().c_str());
    if (trace >= 3) {
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
    }
    fprintf(stderr, "\n");
}

static bool initialized;

void VM::init() {
    if (!initialized) {
        for (int i = 0; i < 256; i++) {
            int n = 0;
            for (int j = 1; j < 256; j += j) {
                if (i & j) n++;
            }
            ptable[i] = (n & 1) == 0;
        }
        initialized = true;
    }
    uint16_t tmp = 0x1234;
    uint8_t *p = (uint8_t *) r;
    if (*(uint8_t *) & tmp == 0x34) {
        for (int i = 0; i < 4; i++) {
            r8[i] = p + i * 2;
            r8[i + 4] = r8[i] + 1;
        }
    } else {
        for (int i = 0; i < 4; i++) {
            r8[i] = p + i * 2 + 1;
            r8[i + 4] = r8[i] - 1;
        }
    }
}

VM::VM() : ip(0), start_sp(0) {
    init();
    memset(r, 0, sizeof (r));
    OF = DF = SF = ZF = PF = CF = false;
}

VM::VM(const VM &vm) : VMUnix(vm) {
    init();
    memcpy(r, vm.r, sizeof (r));
    ip = vm.ip;
    OF = vm.OF;
    DF = vm.DF;
    SF = vm.SF;
    ZF = vm.ZF;
    PF = vm.PF;
    CF = vm.CF;
    start_sp = vm.start_sp;
    cache = vm.cache;
}

VM::~VM() {
}

int VM::addr(const Operand &opr) {
    switch (opr.type) {
        case Ptr: return uint16_t(opr.value);
        case ModRM + 0: return uint16_t(BX + SI + opr.value);
        case ModRM + 1: return uint16_t(BX + DI + opr.value);
        case ModRM + 2: return uint16_t(BP + SI + opr.value);
        case ModRM + 3: return uint16_t(BP + DI + opr.value);
        case ModRM + 4: return uint16_t(SI + opr.value);
        case ModRM + 5: return uint16_t(DI + opr.value);
        case ModRM + 6: return uint16_t(BP + opr.value);
        case ModRM + 7: return uint16_t(BX + opr.value);
    }
    return -1;
}

uint8_t VM::get8(const Operand &opr) {
    switch (opr.type) {
        case Reg: return *r8[opr.value];
        case Imm: return opr.value;
    }
    int ad = addr(opr);
    return ad < 0 ? 0 : read8(ad);
}

uint16_t VM::get16(const Operand &opr) {
    switch (opr.type) {
        case Reg: return r[opr.value];
        case Imm: return opr.value;
    }
    int ad = addr(opr);
    return ad < 0 ? 0 : read16(ad);
}

void VM::set8(const Operand &opr, uint8_t value) {
    if (opr.type == Reg) {
        *r8[opr.value] = value;
    } else {
        int ad = addr(opr);
        if (ad >= 0) write8(ad, value);
    }
}

void VM::set16(const Operand &opr, uint16_t value) {
    if (opr.type == Reg) {
        r[opr.value] = value;
    } else {
        int ad = addr(opr);
        if (ad >= 0) write16(ad, value);
    }
}

void VM::run(
        const std::vector<std::string> &args,
        const std::vector<std::string> &envs) {
    if (trace >= 2) fprintf(stderr, header);
    int slen = 0;
    for (int i = 0; i < (int) args.size(); i++) {
        slen += args[i].size() + 1;
    }
    for (int i = 0; i < (int) envs.size(); i++) {
        slen += envs[i].size() + 1;
    }
    SP -= (slen + 1) & ~1;
    uint16_t ad1 = SP;
    SP -= (1 + args.size() + 1 + envs.size() + 1) * 2;
    uint16_t ad2 = start_sp = SP;
    write16(SP, args.size()); // argc
    for (int i = 0; i < (int) args.size(); i++) {
        write16(ad2 += 2, ad1);
        strcpy((char *) data + ad1, args[i].c_str());
        ad1 += args[i].size() + 1;
    }
    write16(ad2 += 2, 0); // argv[argc]
    for (int i = 0; i < (int) envs.size(); i++) {
        write16(ad2 += 2, ad1);
        strcpy((char *) data + ad1, envs[i].c_str());
        ad1 += envs[i].size() + 1;
    }
    write16(ad2 += 2, 0); // envp (last)
    run();
}

void VM::run() {
    VMUnix *from = current;
    swtch(this);
    hasExited = false;
    while (!hasExited) run1();
    swtch(from);
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
    if (tsize >= 0x20) {
        uint8_t h[0x20];
        if (fread(h, sizeof (h), 1, f) && h[0] == 1 && h[1] == 3
                && !fseek(f, h[4], SEEK_SET)) {
            if (h[3] != 4) {
                fprintf(stderr, "unknown cpu id: %d\n", h[3]);
                fclose(f);
                return false;
            }
            tsize = ::read32(h + 8);
            dsize = ::read32(h + 12);
            ip = ::read32(h + 20);
            if (h[2] & 0x20) {
                cache.clear();
                cache.resize(0x10000);
                data = new uint8_t[0x10000];
                memset(data, 0, 0x10000);
                fread(text, 1, tsize, f);
                fread(data, 1, dsize, f);
            } else {
                cache.clear();
                data = text;
                dsize += tsize;
                fread(text, 1, dsize, f);
            }
            dsize += ::read32(h + 16); // bss
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

void VM::disasm() {
    ::disasm(text, tsize);
}

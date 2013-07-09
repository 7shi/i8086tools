#include "VM.h"
#include "../PDP11/regs.h"
#include <string.h>

using namespace UnixV6;

bool UnixV6::check(uint8_t h[2]) {
    int magic = ::read16(h);
    return magic == 0407 || magic == 0410 || magic == 0411;
}

VM::VM() {
}

VM::VM(const VM &vm) : PDP11::VM(vm) {
}

VM::~VM() {
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

bool VM::load2(const std::string &fn, FILE *f) {
    if (tsize < 0x10) return PDP11::VM::load2(fn, f);

    uint8_t h[0x10];
    fread(h, sizeof (h), 1, f);
    if (!check(h)) {
        fseek(f, 0, SEEK_SET);
        return PDP11::VM::load2(fn, f);
    }

    tsize = ::read16(h + 2);
    dsize = ::read16(h + 4);
    uint16_t bss = ::read16(h + 6);
    PC = ::read16(h + 10);
    cache.clear();
    cache.resize(0x10000);
    if (h[0] == 9) { // 0411
        data = new uint8_t[0x10000];
        memset(data, 0, 0x10000);
        fread(text, 1, tsize, f);
        fread(data, 1, dsize, f);
        runmax = tsize;
        brksize = dsize + bss;
    } else if (h[0] == 8) { // 0410
        data = text;
        fread(text, 1, tsize, f);
        uint16_t doff = (tsize + 0x1fff) & ~0x1fff;
        fread(text + doff, 1, dsize, f);
        runmax = tsize;
        brksize = doff + dsize + bss;
    } else { // 0407
        data = text;
        runmax = tsize + dsize; // for as
        fread(text, 1, runmax, f);
        brksize = runmax + bss;
    }

    uint16_t ssize = ::read16(h + 8);
    if (!ssize) return true;

    if (!::read16(h + 14)) {
        fseek(f, tsize + dsize, SEEK_CUR);
    }
    uint8_t buf[12];
    for (int i = 0; i < ssize; i += 12) {
        fread(buf, sizeof (buf), 1, f);
        PDP11::Symbol sym = {
            readstr(buf, 8), ::read16(buf + 8), ::read16(buf + 10)
        };
        int t = sym.type;
        if (t < 6) {
            t = "uatdbc"[t];
        } else if (0x20 <= t && t < 0x26) {
            t = "UATDBC"[t - 0x20];
        }
        switch (t) {
            case 't':
            case 'T':
                if (!startsWith(sym.name, "~")) {
                    syms[1][sym.addr] = sym;
                }
                break;
            case 0x1f:
                syms[0][sym.addr] = sym;
                break;
        }
    }
    return true;
}

void VM::setstat(uint16_t addr, struct stat * st) {
    memset(data + addr, 0, 36);
    write16(addr, st->st_dev);
    write16(addr + 2, st->st_ino);
    write16(addr + 4, st->st_mode);
    write8(addr + 6, st->st_nlink);
    write8(addr + 7, st->st_uid);
    write8(addr + 8, st->st_gid);
    write8(addr + 9, st->st_size >> 16);
    write16(addr + 10, st->st_size);
    write16(addr + 28, st->st_atime >> 16);
    write16(addr + 30, st->st_atime);
    write16(addr + 32, st->st_mtime >> 16);
    write16(addr + 34, st->st_mtime);
}

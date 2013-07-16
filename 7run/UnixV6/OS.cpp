#include "OS.h"
#include "../PDP11/regs.h"
#include <string.h>

using namespace UnixV6;

bool UnixV6::check(uint8_t h[2]) {
    int magic = ::read16(h);
    return magic == 0407 || magic == 0410 || magic == 0411;
}

OS::OS() {
    vmbase = &cpu;
    cpu.unix = this;
}

OS::OS(const OS &vm) : UnixBase(vm), cpu(vm.cpu) {
    vmbase = &cpu;
    cpu.unix = this;
}

OS::~OS() {
}

void OS::disasm() {
    cpu.disasm();
}

void OS::setArgs(
        const std::vector<std::string> &args,
        const std::vector<std::string> &envs) {
    int slen = 0;
    for (int i = 0; i < (int) args.size(); i++) {
        slen += args[i].size() + 1;
    }
    cpu.SP -= (slen + 1) & ~1;
    uint16_t ad1 = cpu.SP;
    cpu.SP -= (1 + args.size()) * 2;
    uint16_t ad2 = cpu.start_sp = cpu.SP;
    cpu.write16(cpu.SP, args.size()); // argc
    for (int i = 0; i < (int) args.size(); i++) {
        cpu.write16(ad2 += 2, ad1);
        strcpy((char *) cpu.data + ad1, args[i].c_str());
        ad1 += args[i].size() + 1;
    }
}

bool OS::load2(const std::string &fn, FILE *f, size_t size) {
    uint8_t h[0x10];
    if (!fread(h, sizeof (h), 1, f) || !check(h)) {
        if (size > 0xffff) {
            fprintf(stderr, "too long raw binary: %s\n", fn.c_str());
            return false;
        }
        fseek(f, 0, SEEK_SET);
        fread(cpu.text, 1, size, f);
        cpu.PC = 0;
        cpu.cache.clear();
        cpu.data = cpu.text;
        cpu.tsize = cpu.runmax = cpu.brksize = cpu.tsize;
        cpu.dsize = 0;
        return true;
    }

    cpu.tsize = ::read16(h + 2);
    cpu.dsize = ::read16(h + 4);
    uint16_t bss = ::read16(h + 6);
    cpu.PC = ::read16(h + 10);
    cpu.cache.clear();
    cpu.cache.resize(0x10000);
    if (h[0] == 9) { // 0411
        cpu.data = new uint8_t[0x10000];
        memset(cpu.data, 0, 0x10000);
        fread(cpu.text, 1, cpu.tsize, f);
        fread(cpu.data, 1, cpu.dsize, f);
        cpu.runmax = cpu.tsize;
        cpu.brksize = cpu.dsize + bss;
    } else if (h[0] == 8) { // 0410
        cpu.data = cpu.text;
        fread(cpu.text, 1, cpu.tsize, f);
        uint16_t doff = (cpu.tsize + 0x1fff) & ~0x1fff;
        fread(cpu.text + doff, 1, cpu.dsize, f);
        cpu.runmax = cpu.tsize;
        cpu.brksize = doff + cpu.dsize + bss;
    } else { // 0407
        cpu.data = cpu.text;
        cpu.runmax = cpu.tsize + cpu.dsize; // for as
        fread(cpu.text, 1, cpu.runmax, f);
        cpu.brksize = cpu.runmax + bss;
    }

    uint16_t ssize = ::read16(h + 8);
    if (!ssize) return true;

    if (!::read16(h + 14)) {
        fseek(f, cpu.tsize + cpu.dsize, SEEK_CUR);
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
                    cpu.syms[1][sym.addr] = sym;
                }
                break;
            case 0x1f:
                cpu.syms[0][sym.addr] = sym;
                break;
        }
    }
    return true;
}

void OS::setstat(uint16_t addr, struct stat * st) {
    memset(cpu.data + addr, 0, 36);
    cpu.write16(addr, st->st_dev);
    cpu.write16(addr + 2, st->st_ino);
    cpu.write16(addr + 4, st->st_mode);
    cpu.write8(addr + 6, st->st_nlink);
    cpu.write8(addr + 7, st->st_uid);
    cpu.write8(addr + 8, st->st_gid);
    cpu.write8(addr + 9, st->st_size >> 16);
    cpu.write16(addr + 10, st->st_size);
    cpu.write16(addr + 28, st->st_atime >> 16);
    cpu.write16(addr + 30, st->st_atime);
    cpu.write16(addr + 32, st->st_mtime >> 16);
    cpu.write16(addr + 34, st->st_mtime);
}

#include "VM.h"
#include "../i8086/regs.h"
#include <string.h>

using namespace Minix2;

VM::VM() {
    memset(sigacts, 0, sizeof (sigacts));
}

VM::VM(const VM &vm) : i8086::VM(vm) {
    memcpy(sigacts, vm.sigacts, sizeof (sigacts));
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
}

bool VM::load2(const std::string &fn, FILE *f) {
    if (tsize < 0x20) return i8086::VM::load2(fn, f);
    uint8_t h[0x20];
    if (fread(h, sizeof (h), 1, f) && h[0] == 1 && h[1] == 3
            && !fseek(f, h[4], SEEK_SET)) {
        if (h[3] != 4) {
            fprintf(stderr, "unknown cpu id: %d\n", h[3]);
            return false;
        }
        tsize = ::read32(h + 8);
        dsize = ::read32(h + 12);
        uint16_t bss = ::read32(h + 16);
        ip = ::read32(h + 20);
        cache.clear();
        if (h[2] & 0x20) {
            cache.resize(0x10000);
            data = new uint8_t[0x10000];
            memset(data, 0, 0x10000);
            fread(text, 1, tsize, f);
            fread(data, 1, dsize, f);
            brksize = dsize + bss;
        } else {
            data = text;
            fread(text, 1, tsize + dsize, f);
            brksize = tsize + dsize + bss;
        }
        return true;
    }
    fseek(f, 0, SEEK_SET);
    return i8086::VM::load2(fn, f);
}

void VM::setstat(uint16_t addr, struct stat *st) {
    write16(addr, st->st_dev);
    write16(addr + 2, st->st_ino);
    write16(addr + 4, st->st_mode);
    write16(addr + 6, st->st_nlink);
    write16(addr + 8, st->st_uid);
    write16(addr + 10, st->st_gid);
    write16(addr + 12, st->st_rdev);
    write32(addr + 14, st->st_size);
    write32(addr + 18, st->st_atime);
    write32(addr + 22, st->st_mtime);
    write32(addr + 26, st->st_ctime);
}

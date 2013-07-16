#include "OS.h"
#include "../i8086/regs.h"
#include <string.h>

using namespace Minix2;

OS::OS() {
    vmbase = &cpu;
    cpu.unix = this;
    memset(sigacts, 0, sizeof (sigacts));
}

OS::OS(const OS &vm) : UnixBase(vm), cpu(vm.cpu) {
    vmbase = &cpu;
    cpu.unix = this;
    memcpy(sigacts, vm.sigacts, sizeof (sigacts));
}

OS::~OS() {
}

void OS::disasm() {
    vmbase->disasm();
}

void OS::setArgs(
        const std::vector<std::string> &args,
        const std::vector<std::string> &envs) {
    int slen = 0;
    for (int i = 0; i < (int) args.size(); i++) {
        slen += args[i].size() + 1;
    }
    for (int i = 0; i < (int) envs.size(); i++) {
        slen += envs[i].size() + 1;
    }
    cpu.SP -= (slen + 1) & ~1;
    uint16_t ad1 = cpu.SP;
    cpu.SP -= (1 + args.size() + 1 + envs.size() + 1) * 2;
    uint16_t ad2 = cpu.start_sp = cpu.SP;
    vmbase->write16(cpu.SP, args.size()); // argc
    for (int i = 0; i < (int) args.size(); i++) {
        vmbase->write16(ad2 += 2, ad1);
        strcpy((char *) vmbase->data + ad1, args[i].c_str());
        ad1 += args[i].size() + 1;
    }
    vmbase->write16(ad2 += 2, 0); // argv[argc]
    for (int i = 0; i < (int) envs.size(); i++) {
        vmbase->write16(ad2 += 2, ad1);
        strcpy((char *) vmbase->data + ad1, envs[i].c_str());
        ad1 += envs[i].size() + 1;
    }
    vmbase->write16(ad2 += 2, 0); // envp (last)
}

bool OS::load2(const std::string &fn, FILE *f, size_t size) {
    uint8_t h[0x20];
    if (!fread(h, sizeof (h), 1, f) || !(h[0] == 1 && h[1] == 3)) {
        return vmbase->load(fn, f, size);
    }
    if (h[3] != 4) {
        fprintf(stderr, "unknown cpu id: %d\n", h[3]);
        return false;
    }
    vmbase->release();
    vmbase->text = new uint8_t[0x10000];
    memset(vmbase->text, 0, 0x10000);
    fseek(f, h[4], SEEK_SET);
    vmbase->tsize = ::read32(h + 8);
    vmbase->dsize = ::read32(h + 12);
    uint16_t bss = ::read32(h + 16);
    cpu.ip = ::read32(h + 20);
    cpu.cache.clear();
    if (h[2] & 0x20) {
        cpu.cache.resize(0x10000);
        vmbase->data = new uint8_t[0x10000];
        memset(vmbase->data, 0, 0x10000);
        fread(vmbase->text, 1, vmbase->tsize, f);
        fread(vmbase->data, 1, vmbase->dsize, f);
        vmbase->brksize = vmbase->dsize + bss;
    } else {
        vmbase->data = vmbase->text;
        fread(vmbase->text, 1, vmbase->tsize + vmbase->dsize, f);
        vmbase->brksize = vmbase->tsize + vmbase->dsize + bss;
    }
    return true;
}

void OS::setstat(uint16_t addr, struct stat *st) {
    vmbase->write16(addr, st->st_dev);
    vmbase->write16(addr + 2, st->st_ino);
    vmbase->write16(addr + 4, st->st_mode);
    vmbase->write16(addr + 6, st->st_nlink);
    vmbase->write16(addr + 8, st->st_uid);
    vmbase->write16(addr + 10, st->st_gid);
    vmbase->write16(addr + 12, st->st_rdev);
    vmbase->write32(addr + 14, st->st_size);
    vmbase->write32(addr + 18, st->st_atime);
    vmbase->write32(addr + 22, st->st_mtime);
    vmbase->write32(addr + 26, st->st_ctime);
}

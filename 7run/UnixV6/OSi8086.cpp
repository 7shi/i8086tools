#include "OSi8086.h"
#include "../i8086/regs.h"
#include <string.h>

using namespace UnixV6;

bool OSi8086::check(uint8_t h[2]) {
    return h[0] == 0xeb && (h[1] == 0x0e || h[1] == 0x10 || h[1] == 0x12);
}

OSi8086::OSi8086() {
    vm = &cpu;
    cpu.unix = this;
}

OSi8086::OSi8086(const OSi8086 &os) : OS(os), cpu(os.cpu) {
    vm = &cpu;
    cpu.unix = this;
}

OSi8086::~OSi8086() {
}

void OSi8086::disasm() {
    vm->disasm();
}

void OSi8086::setArgs(
        const std::vector<std::string> &args,
        const std::vector<std::string> &) {
    int slen = 0;
    for (int i = 0; i < (int) args.size(); i++) {
        slen += args[i].size() + 1;
    }
    cpu.SP -= (slen + 1) & ~1;
    uint16_t ad1 = cpu.SP;
    cpu.SP -= (1 + args.size()) * 2;
    uint16_t ad2 = cpu.start_sp = cpu.SP;
    vm->write16(cpu.SP, args.size()); // argc
    for (int i = 0; i < (int) args.size(); i++) {
        vm->write16(ad2 += 2, ad1);
        strcpy((char *) vm->data + ad1, args[i].c_str());
        ad1 += args[i].size() + 1;
    }
}

bool OSi8086::load2(const std::string &fn, FILE *f, size_t size) {
    uint8_t h[0x10];
    if (!fread(h, sizeof (h), 1, f) || !check(h)) {
        return vm->load(fn, f, size);
    }

    vm->release();
    vm->text = new uint8_t[0x10000];
    memset(vm->text, 0, 0x10000);
    vm->tsize = ::read16(h + 2);
    vm->dsize = ::read16(h + 4);
    uint16_t bss = ::read16(h + 6);
    cpu.ip = ::read16(h + 10);
    cpu.cache.clear();
    cpu.cache.resize(0x10000);
    if (h[1] == 0x12) { // 0411
        vm->data = new uint8_t[0x10000];
        memset(vm->data, 0, 0x10000);
        fread(vm->text, 1, vm->tsize, f);
        fread(vm->data, 1, vm->dsize, f);
        vm->brksize = vm->dsize + bss;
    } else if (h[0] == 0x10) { // 0410
        vm->data = vm->text;
        fread(vm->text, 1, vm->tsize, f);
        uint16_t doff = (vm->tsize + 0x1fff) & ~0x1fff;
        fread(vm->text + doff, 1, vm->dsize, f);
        vm->brksize = doff + vm->dsize + bss;
    } else { // 0407
        vm->data = vm->text;
        int rlen = vm->tsize + vm->dsize;
        fread(vm->text, 1, rlen, f);
        vm->brksize = rlen + bss;
    }

    uint16_t ssize = ::read16(h + 8);
    if (ssize) {
        if (!::read16(h + 14)) {
            fseek(f, vm->tsize + vm->dsize, SEEK_CUR);
        }
        readsym(f, ssize);
    }
    return true;
}

bool OSi8086::syscall(int n) {
    int result, ret = OS::syscall(&result, n, cpu.AX, vm->text + cpu.ip);
    if (ret >= 0) {
        cpu.ip += ret;
        cpu.AX = (cpu.CF = (result == -1)) ? errno : result;
    }
    return true;
}

int OSi8086::v6_fork() { // 2
    if (trace) fprintf(stderr, "<fork()>\n");
#ifdef NO_FORK
    OSi8086 vm = *this;
    vm.run();
    return vm.pid;
#else
    int result = fork();
    return result <= 0 ? result : (result % 30000) + 1;
#endif
}

int OSi8086::v6_wait() { // 7
    int status, result = sys_wait(&status);
    cpu.DX = status | 14;
    return result;
}

int OSi8086::v6_exec(const char *path, int argp) { // 11
#if 0
    FILE *f = fopen("core", "wb");
    fwrite(data, 1, 0x10000, f);
    fclose(f);
#endif
    if (trace) fprintf(stderr, "<exec(\"%s\"", path);
    std::vector<std::string> args, envs;
    int slen = 0, p;
    while ((p = vm->read16(argp + args.size() * 2))) {
        std::string arg = vm->str(p);
        if (trace && !args.empty()) fprintf(stderr, ", \"%s\"", arg.c_str());
        slen += arg.size() + 1;
        args.push_back(arg);
    }
    if (!load(path)) {
        if (trace) fprintf(stderr, ") => EINVAL>\n");
        errno = EINVAL;
        return -1;
    }
    resetsig();
    cpu.SP = 0;
    setArgs(args, envs);
    if (trace) fprintf(stderr, ") => 0>\n");
    return 0;
}

int OSi8086::v6_brk(int nd) { // 17
    return sys_brk(nd, cpu.SP);
}

void OSi8086::sighandler2(int sig) {
    uint16_t r[8];
    memcpy(r, cpu.r, sizeof (r));
    uint16_t ip = cpu.ip;
    bool OF = cpu.OF, DF = cpu.DF, SF = cpu.SF;
    bool ZF = cpu.ZF, PF = cpu.PF, CF = cpu.CF;
    cpu.write16((cpu.SP -= 2), cpu.ip);
    cpu.ip = sighandlers[sig];
    while (!cpu.hasExited && !(cpu.ip == ip && cpu.SP == SP)) {
        cpu.run1();
    }
    if (!cpu.hasExited) {
        memcpy(cpu.r, r, sizeof (r));
        cpu.OF = OF;
        cpu.DF = DF;
        cpu.SF = SF;
        cpu.ZF = ZF;
        cpu.PF = PF;
        cpu.CF = CF;
    }
}

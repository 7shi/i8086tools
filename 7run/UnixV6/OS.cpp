#include "OS.h"
#include "../PDP11/regs.h"
#include <string.h>

using namespace UnixV6;

bool UnixV6::check(uint8_t h[2]) {
    int magic = ::read16(h);
    return magic == 0407 || magic == 0410 || magic == 0411;
}

OS::OS() {
    vm = &cpu;
    cpu.unix = this;
}

OS::OS(const OS &os) : OSBase(os), cpu(os.cpu) {
    vm = &cpu;
    cpu.unix = this;
}

OS::~OS() {
}

void OS::setArgs(
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

bool OS::load2(const std::string &fn, FILE *f, size_t size) {
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
    cpu.PC = ::read16(h + 10);
    cpu.cache.clear();
    cpu.cache.resize(0x10000);
    if (h[0] == 9) { // 0411
        vm->data = new uint8_t[0x10000];
        memset(vm->data, 0, 0x10000);
        fread(vm->text, 1, vm->tsize, f);
        fread(vm->data, 1, vm->dsize, f);
        cpu.runmax = vm->tsize;
        vm->brksize = vm->dsize + bss;
    } else if (h[0] == 8) { // 0410
        vm->data = vm->text;
        fread(vm->text, 1, vm->tsize, f);
        uint16_t doff = (vm->tsize + 0x1fff) & ~0x1fff;
        fread(vm->text + doff, 1, vm->dsize, f);
        cpu.runmax = vm->tsize;
        vm->brksize = doff + vm->dsize + bss;
    } else { // 0407
        vm->data = vm->text;
        cpu.runmax = vm->tsize + vm->dsize; // for as
        fread(vm->text, 1, cpu.runmax, f);
        vm->brksize = cpu.runmax + bss;
    }

    uint16_t ssize = ::read16(h + 8);
    if (!ssize) return true;

    if (!::read16(h + 14)) {
        fseek(f, vm->tsize + vm->dsize, SEEK_CUR);
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

bool OS::syscall(int n) {
    int result, ret = OSBase::syscall(&result, n, cpu.r[0], cpu.text + cpu.PC);
    if (ret >= 0) {
        cpu.PC += ret;
        cpu.r[0] = (cpu.C = (result == -1)) ? errno : result;
    }
    return true;
}

int OS::v6_fork() { // 2
    if (trace) fprintf(stderr, "<fork()>\n");
#ifdef NO_FORK
    OS vm = *this;
    vm.run();
    return vm.pid;
#else
    int result = fork();
    return result <= 0 ? result : (result % 30000) + 1;
#endif
}

int OS::v6_wait() { // 7
    int status, result = sys_wait(&status);
    cpu.r[1] = status | 14;
    return result;
}

int OS::v6_exec(const char *path, int argp) { // 11
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

int OS::v6_brk(int nd) { // 17
    return sys_brk(nd, cpu.SP);
}

void OS::sighandler2(int sig) {
    uint16_t r[8];
    memcpy(r, cpu.r, sizeof (r));
    bool Z = cpu.Z, N = cpu.N, C = cpu.C, V = cpu.V;
    cpu.write16((cpu.SP -= 2), cpu.PC);
    cpu.PC = sighandlers[sig];
    while (!cpu.hasExited && !(cpu.PC == PC && cpu.SP == SP)) {
        cpu.run1();
    }
    if (!cpu.hasExited) {
        memcpy(cpu.r, r, sizeof (r));
        cpu.Z = Z;
        cpu.N = N;
        cpu.C = C;
        cpu.V = V;
    }
}

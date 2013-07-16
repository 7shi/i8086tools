#include "OS.h"
#include "../PDP11/regs.h"
#include <string.h>

using namespace UnixV6;

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

#include "VM.h"
#include "../PDP11/regs.h"
#include <stdio.h>

using namespace UnixV6;

VM::VM() {
}

VM::~VM() {
}

void VM::setstat(uint16_t addr, struct stat *st) {
    fprintf(stderr, "[%s] not implemented\n", __func__);
}

bool VM::syscall(int n) {
    return syscall(n, text + PC);
}

bool VM::syscall(int n, uint8_t *args) {
    int result = 0;
    switch (n) {
        case 0:
        {
            int bak = PC + 2;
            int p = ::read16(text + PC);
            bool ret = syscall(read8(p), data + p + 2);
            PC = bak;
            return ret;
        }
        case 1:
            fprintf(stderr, "<exit: not implemented>\n");
            hasExited = true;
            break;
        case 2:
            fprintf(stderr, "<fork: not implemented>\n");
            hasExited = true;
            break;
        case 3:
            fprintf(stderr, "<read: not implemented>\n");
            hasExited = true;
            break;
        case 4:
            fprintf(stderr, "<write: not implemented>\n");
            hasExited = true;
            break;
        case 5:
            fprintf(stderr, "<open: not implemented>\n");
            hasExited = true;
            break;
        case 6:
            fprintf(stderr, "<close: not implemented>\n");
            hasExited = true;
            break;
        case 7:
            fprintf(stderr, "<wait: not implemented>\n");
            hasExited = true;
            break;
        case 8:
            fprintf(stderr, "<creat: not implemented>\n");
            hasExited = true;
            break;
        case 9:
            fprintf(stderr, "<link: not implemented>\n");
            hasExited = true;
            break;
        case 10:
            fprintf(stderr, "<unlink: not implemented>\n");
            hasExited = true;
            break;
        case 11:
            fprintf(stderr, "<exec: not implemented>\n");
            hasExited = true;
            break;
        case 12:
            fprintf(stderr, "<chdir: not implemented>\n");
            hasExited = true;
            break;
        case 13:
            fprintf(stderr, "<time: not implemented>\n");
            hasExited = true;
            break;
        case 14:
            fprintf(stderr, "<mknod: not implemented>\n");
            hasExited = true;
            break;
        case 15:
            fprintf(stderr, "<chmod: not implemented>\n");
            hasExited = true;
            break;
        case 16:
            fprintf(stderr, "<chown: not implemented>\n");
            hasExited = true;
            break;
        case 17:
            fprintf(stderr, "<break: not implemented>\n");
            hasExited = true;
            break;
        case 18:
            fprintf(stderr, "<stat: not implemented>\n");
            hasExited = true;
            break;
        case 19:
            fprintf(stderr, "<seek: not implemented>\n");
            hasExited = true;
            break;
        case 20:
            fprintf(stderr, "<getpid: not implemented>\n");
            hasExited = true;
            break;
        case 21:
            fprintf(stderr, "<mount: not implemented>\n");
            hasExited = true;
            break;
        case 22:
            fprintf(stderr, "<umount: not implemented>\n");
            hasExited = true;
            break;
        case 23:
            fprintf(stderr, "<setuid: not implemented>\n");
            hasExited = true;
            break;
        case 24:
            fprintf(stderr, "<getuid: not implemented>\n");
            hasExited = true;
            break;
        case 25:
            fprintf(stderr, "<stime: not implemented>\n");
            hasExited = true;
            break;
        case 26:
            fprintf(stderr, "<ptrace: not implemented>\n");
            hasExited = true;
            break;
        case 28:
            fprintf(stderr, "<fstat: not implemented>\n");
            hasExited = true;
            break;
        case 31:
            fprintf(stderr, "<stty: not implemented>\n");
            hasExited = true;
            break;
        case 32:
            fprintf(stderr, "<gtty: not implemented>\n");
            hasExited = true;
            break;
        case 34:
            fprintf(stderr, "<nice: not implemented>\n");
            hasExited = true;
            break;
        case 35:
            fprintf(stderr, "<sleep: not implemented>\n");
            hasExited = true;
            break;
        case 36:
            fprintf(stderr, "<sync: not implemented>\n");
            hasExited = true;
            break;
        case 37:
            fprintf(stderr, "<kill: not implemented>\n");
            hasExited = true;
            break;
        case 38:
            fprintf(stderr, "<switch: not implemented>\n");
            hasExited = true;
            break;
        case 41:
            fprintf(stderr, "<dup: not implemented>\n");
            hasExited = true;
            break;
        case 42:
            fprintf(stderr, "<pipe: not implemented>\n");
            hasExited = true;
            break;
        case 43:
            fprintf(stderr, "<times: not implemented>\n");
            hasExited = true;
            break;
        case 44:
            fprintf(stderr, "<prof: not implemented>\n");
            hasExited = true;
            break;
        case 46:
            fprintf(stderr, "<setgid: not implemented>\n");
            hasExited = true;
            break;
        case 47:
            fprintf(stderr, "<getgid: not implemented>\n");
            hasExited = true;
            break;
        case 48:
            fprintf(stderr, "<signal: not implemented>\n");
            hasExited = true;
            break;
    }
    r[0] = (C = (result == -1)) ? errno : result;
    return true;
}

int VM::convsig(int sig) {
    fprintf(stderr, "[%s] not implemented\n", __func__);
    return 0;
}

void VM::setsig(int sig, int h) {
    fprintf(stderr, "[%s] not implemented\n", __func__);
}

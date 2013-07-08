#include "VM.h"
#include "../PDP11/regs.h"
#include <stdio.h>
#include <string.h>

using namespace UnixV6;

bool UnixV6::check(uint8_t h[2]) {
    int magic = ::read16(h);
    return magic == 0407 || magic == 0410 || magic == 0411;
}

VM::VM() {
}

VM::~VM() {
}

bool VM::loadInternal(const std::string &fn, FILE *f) {
    if (tsize < 0x10) return PDP11::VM::loadInternal(fn, f);
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
                dsize = (tsize += dsize);
                fread(text, 1, dsize, f);
            }
        }
        dsize += ::read16(h + 6); // bss
        brksize = dsize;
        return true;
    }
    fseek(f, 0, SEEK_SET);
    return PDP11::VM::loadInternal(fn, f);
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
            sys_exit((int16_t) r[0]);
            return true;
        case 2:
            fprintf(stderr, "<fork: not implemented>\n");
            hasExited = true;
            break;
        case 3:
            fprintf(stderr, "<read: not implemented>\n");
            hasExited = true;
            break;
        case 4:
            PC += 4;
            result = sys_write(r[0], ::read16(args), ::read16(args + 2));
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

void VM::swtch(bool reset) {
    fprintf(stderr, "[%s] not implemented\n", __func__);
}

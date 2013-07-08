#include "VM.h"
#include "../PDP11/regs.h"
#include <string.h>

using namespace UnixV6;

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
            int nn = read8(p);
            bool ret = syscall(nn, data + p + 2);
            if (!(nn == 11 && !C)) PC = bak;
            return ret;
        }
        case 1:
            sys_exit((int16_t) r[0]);
            return true;
        case 2:
            result = v6_fork();
            break;
        case 3:
            PC += 4;
            result = sys_read(r[0], ::read16(args), ::read16(args + 2));
            break;
        case 4:
            PC += 4;
            result = sys_write(r[0], ::read16(args), ::read16(args + 2));
            break;
        case 5:
            PC += 4;
            result = sys_open(str(::read16(args)), ::read16(args + 2));
            break;
        case 6:
            result = sys_close(r[0]);
            break;
        case 7:
        {
            int status;
            result = sys_wait(&status);
            r[1] = status | 14;
            break;
        }
        case 8:
            PC += 4;
            result = sys_creat(str(::read16(args)), ::read16(args + 2));
            break;
        case 9:
            PC += 4;
            result = sys_link(str(::read16(args)), str(::read16(args + 2)));
            break;
        case 10:
            PC += 2;
            result = sys_unlink(str(::read16(args)));
            break;
        case 11:
            PC += 4;
            result = v6_exec(str(::read16(args)), ::read16(args + 2));
            break;
        case 12:
            PC += 2;
            result = sys_chdir(str(::read16(args)));
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
            PC += 4;
            result = sys_chmod(str(::read16(args)), ::read16(args + 2));
            break;
        case 16:
            fprintf(stderr, "<chown: not implemented>\n");
            hasExited = true;
            break;
        case 17:
            PC += 2;
            result = sys_brk(::read16(args), SP);
            break;
        case 18:
            PC += 4;
            result = sys_stat(str(::read16(args)), ::read16(args + 2));
            break;
        case 19:
            PC += 4;
            result = v6_seek(r[0], ::read16(args), ::read16(args + 2));
            break;
        case 20:
            result = sys_getpid();
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
            //result = dup(r[0]);
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
            PC += 4;
            result = v6_signal(::read16(args), ::read16(args + 2));
            break;
    }
    r[0] = (C = (result == -1)) ? errno : result;
    return true;
}

int VM::v6_fork() { // 2
    if (trace) fprintf(stderr, "<fork()>\n");
#ifdef NO_FORK
    VM vm = *this;
    vm.run();
    PC += 2;
    return vm.pid;
#else
    int result = fork();
    return result <= 0 ? result : (result % 30000) + 1;
#endif
}

int VM::v6_exec(const char *path, int args) { // 11
#if 0
    FILE *f = fopen("core", "wb");
    fwrite(data, 1, 0x10000, f);
    fclose(f);
#endif
    if (trace) fprintf(stderr, "<exec(\"%s\"", path);
    int argc = 0, slen = 0, p;
    for (; (p = read16(args + argc * 2)); argc++) {
        const char *arg = str(p);
        if (trace && argc > 0) fprintf(stderr, ", \"%s\"", arg);
        slen += strlen(arg) + 1;
    }
    if (trace) fprintf(stderr, ")>\n");
    uint8_t *t = text, *d = data;
    text = new uint8_t[0x10000];
    memset(text, 0, 0x10000);
    data = NULL;
    if (!load(path)) {
        delete[] text;
        text = t;
        data = d;
        errno = EINVAL;
        return -1;
    }
    //resetsig();
    SP = 0x10000 - ((slen + 1) & ~1);
    uint16_t ad1 = SP;
    SP -= (argc + 1) * 2;
    uint16_t ad2 = start_sp = SP;
    write16(SP, argc);
    for (int i = 0; i < argc; i++) {
        write16(ad2 += 2, ad1);
        const char *arg = (const char *) (d + ::read16(d + args + i * 2));
        strcpy((char *) data + ad1, arg);
        ad1 += strlen(arg) + 1;
    }
    if (d != t) delete[] d;
    return 0;
}

int VM::v6_seek(int fd, off_t o, int w) { // 19
    if (trace) fprintf(stderr, "<lseek(%d, %ld, %d)", fd, o, w);
    FileBase *f = file(fd);
    off_t result = -1;
    switch (w) {
        case 0:
            result = f->lseek(o, SEEK_SET);
            break;
        case 1:
            result = f->lseek(int16_t(uint16_t(o)), SEEK_CUR);
            break;
        case 2:
            result = f->lseek(int16_t(uint16_t(o)), SEEK_END);
            break;
        case 3:
            result = f->lseek(o * 512, SEEK_SET);
            break;
        case 4:
            result = f->lseek(int(int16_t(uint16_t(o))) * 512, SEEK_CUR);
            break;
        case 5:
            result = f->lseek(int(int16_t(uint16_t(o))) * 512, SEEK_END);
            break;
        default:
            errno = EINVAL;
            break;
    }
    if (trace) fprintf(stderr, " => %ld>\n", result);
    return result;
}

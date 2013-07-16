#include "OS.h"
#include "../i8086/regs.h"
#include <string.h>

using namespace Minix2;

#define str cpu.str
#define hasExited cpu.hasExited

bool OS::syscall(int n) {
    return n == 0x20 ? syscall(cpu.data + cpu.BX) : false;
}

bool OS::syscall(uint8_t *m) {
    int type = read16(m + 2), result = 0;
    switch (type) {
        case 1:
            sys_exit((int16_t) read16(m + 4));
            return true;
        case 2:
            result = minix_fork();
            break;
        case 3:
            result = sys_read(read16(m + 4), read16(m + 10), read16(m + 6));
            break;
        case 4:
            result = sys_write(read16(m + 4), read16(m + 10), read16(m + 6));
            break;
        case 5:
        {
            int flag = read16(m + 6);
            if (flag & 64 /*O_CREAT*/) {
                result = sys_open(str(read16(m + 10)), flag, read16(m + 8));
            } else {
                result = sys_open(str(read16(m + 8)), flag);
            }
            break;
        }
        case 6:
            result = sys_close(read16(m + 4));
            break;
        case 7:
        {
            int status;
            result = sys_wait(&status);
            write16(m + 4, status);
            break;
        }
        case 8:
            result = sys_creat(str(read16(m + 8)), read16(m + 6));
            break;
        case 9:
            result = sys_link(str(read16(m + 10)), str(read16(m + 12)));
            break;
        case 10:
            result = sys_unlink(str(read16(m + 8)));
            break;
        case 11:
            fprintf(stderr, "<waitpid: not implemented>\n");
            hasExited = true;
            break;
        case 12:
            result = sys_chdir(str(read16(m + 8)));
            break;
        case 13:
            result = sys_time();
            if (result >= 0) {
                write32(m + 10, result);
                result = 0;
            }
            break;
        case 14:
            fprintf(stderr, "<mknod: not implemented>\n");
            hasExited = true;
            break;
        case 15:
            result = sys_chmod(str(read16(m + 8)), read16(m + 6));
            break;
        case 16:
            fprintf(stderr, "<chown: not implemented>\n");
            hasExited = true;
            break;
        case 17:
            result = sys_brk(read16(m + 10), cpu.SP);
            if (!result) write16(m + 18, cpu.brksize);
            break;
        case 18:
            result = sys_stat(str(read16(m + 10)), read16(m + 12));
            break;
        case 19:
        {
            off_t o = sys_lseek(read16(m + 4), read32(m + 10), read16(m + 6));
            if (o == -1) {
                result = -1;
            } else {
                write32(m + 10, o);
            }
            break;
        }
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
            result = sys_getuid();
            break;
        case 25:
            fprintf(stderr, "<stime: not implemented>\n");
            hasExited = true;
            break;
        case 26:
            fprintf(stderr, "<ptrace: not implemented>\n");
            hasExited = true;
            break;
        case 27:
            fprintf(stderr, "<alarm: not implemented>\n");
            hasExited = true;
            break;
        case 28:
            result = sys_fstat(read16(m + 4), read16(m + 10));
            break;
        case 29:
            fprintf(stderr, "<pause: not implemented>\n");
            hasExited = true;
            break;
        case 30:
            fprintf(stderr, "<utime: not implemented>\n");
            hasExited = true;
            break;
        case 33:
            result = sys_access(str(read16(m + 8)), read16(m + 6));
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
            fprintf(stderr, "<rename: not implemented>\n");
            hasExited = true;
            break;
        case 39:
            fprintf(stderr, "<mkdir: not implemented>\n");
            hasExited = true;
            break;
        case 40:
            fprintf(stderr, "<rmdir: not implemented>\n");
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
        case 46:
            fprintf(stderr, "<setgid: not implemented>\n");
            hasExited = true;
            break;
        case 47:
            result = sys_getgid();
            break;
        case 48:
            result = minix_signal(read16(m + 4), read16(m + 14));
            break;
        case 54:
            result = sys_ioctl(read16(m + 4), read16(m + 8), read16(m + 18));
            break;
        case 55:
            fprintf(stderr, "<fcntl: not implemented>\n");
            hasExited = true;
            break;
        case 59:
            if (!minix_exec(str(read16(m + 10)), read16(m + 12), read16(m + 6))) {
                return true;
            }
            break;
        case 60:
            result = sys_umask(read16(m + 4));
            break;
        case 61:
            fprintf(stderr, "<chroot: not implemented>\n");
            hasExited = true;
            break;
        case 62:
            fprintf(stderr, "<setsid: not implemented>\n");
            hasExited = true;
            break;
        case 63:
            fprintf(stderr, "<getpgrp: not implemented>\n");
            hasExited = true;
            break;
        case 64:
            fprintf(stderr, "<ksig: not implemented>\n");
            hasExited = true;
            break;
        case 65:
            fprintf(stderr, "<unpause: not implemented>\n");
            hasExited = true;
            break;
        case 67:
            fprintf(stderr, "<revive: not implemented>\n");
            hasExited = true;
            break;
        case 68:
            fprintf(stderr, "<task_reply: not implemented>\n");
            hasExited = true;
            break;
        case 71:
            result = minix_sigaction(read16(m + 6), read16(m + 10), read16(m + 12));
            break;
        case 72:
            fprintf(stderr, "<sigsuspend: not implemented>\n");
            hasExited = true;
            break;
        case 73:
            fprintf(stderr, "<sigpending: not implemented>\n");
            hasExited = true;
            break;
        case 74:
            fprintf(stderr, "<sigprocmask: not implemented>\n");
            hasExited = true;
            break;
        case 75:
            fprintf(stderr, "<sigreturn: not implemented>\n");
            hasExited = true;
            break;
        case 76:
            fprintf(stderr, "<reboot: not implemented>\n");
            hasExited = true;
            break;
        case 77:
            fprintf(stderr, "<svrctl: not implemented>\n");
            hasExited = true;
            break;
        default:
            fprintf(stderr, "<%d: unknown syscall>\n", type);
            hasExited = true;
            return true;
    }
    write16(m + 2, result == -1 ? -errno : result);
    cpu.AX = 0;
    return true;
}

int OS::minix_fork() { // 2
    if (trace) fprintf(stderr, "<fork()>\n");
#ifdef NO_FORK
    OS vm = *this;
    vm.cpu.write16(cpu.BX + 2, 0);
    vm.cpu.AX = 0;
    vm.run();
    return vm.pid;
#else
    int result = fork();
    return result <= 0 ? result : (result % 30000) + 1;
#endif
}

int OS::minix_exec(const char *path, int frame, int fsize) { // 59
#if 0
    FILE *f = fopen("core", "wb");
    fwrite(data, 1, 0x10000, f);
    fclose(f);
#endif
    std::vector<uint8_t> f(fsize);
    memcpy(&f[0], cpu.data + frame, fsize);
    int argc = read16(&f[0]);
    if (trace) {
        fprintf(stderr, "<exec(\"%s\"", path);
        for (int i = 2; i <= argc; i++) {
            fprintf(stderr, ", \"%s\"", &f[read16(&f[i * 2])]);
        }
        fprintf(stderr, ")>\n");
    }
    if (!load(path)) {
        errno = EINVAL;
        return -1;
    }
    resetsig();
    cpu.start_sp = cpu.SP = 0x10000 - fsize;
    memcpy(cpu.data + cpu.start_sp, &f[0], fsize);
    int ad = cpu.start_sp + 2, p;
    for (int i = 0; i < 2; i++, ad += 2) {
        for (; (p = cpu.read16(ad)); ad += 2) {
            cpu.write16(ad, cpu.start_sp + p);
        }
    }
    return 0;
}

#include "VM.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

using namespace Minix2;

VM::VM() {
}

VM::VM(const VM &vm) : i8086::VM(vm) {
}

VM::~VM() {
}

bool VM::syscall(int n) {
    if (n != 0x20) return false;
    int type = read16(BX + 2), result = 0;
    switch (type) {
        case 1:
            sys_exit((int16_t) read16(BX + 4));
            return true;
        case 2:
            result = minix_fork();
            break;
        case 3:
            result = sys_read(read16(BX + 4), read16(BX + 10), read16(BX + 6));
            break;
        case 4:
            result = sys_write(read16(BX + 4), read16(BX + 10), read16(BX + 6));
            break;
        case 5:
        {
            int flag = read16(BX + 6);
            if (flag & 64 /*O_CREAT*/) {
                result = sys_open(str(read16(BX + 10)), flag, read16(BX + 8));
            } else {
                result = sys_open(str(read16(BX + 8)), flag);
            }
            break;
        }
        case 6:
            result = sys_close(read16(BX + 4));
            break;
        case 7:
        {
            int status;
            result = sys_wait(&status);
            write16(BX + 4, status);
            break;
        }
        case 8:
            result = sys_creat(str(read16(BX + 8)), read16(BX + 6));
            break;
        case 9:
            result = sys_link(str(read16(BX + 10)), str(read16(BX + 12)));
            break;
        case 10:
            result = sys_unlink(str(read16(BX + 8)));
            break;
        case 11:
            fprintf(stderr, "<waitpid: not implemented>\n");
            hasExited = true;
            break;
        case 12:
            fprintf(stderr, "<chdir: not implemented>\n");
            hasExited = true;
            break;
        case 13:
            result = sys_time();
            if (result >= 0) {
                write32(BX + 10, result);
                result = 0;
            }
            break;
        case 14:
            fprintf(stderr, "<mknod: not implemented>\n");
            hasExited = true;
            break;
        case 15:
            result = sys_chmod(str(read16(BX + 8)), read16(BX + 6));
            break;
        case 16:
            fprintf(stderr, "<chown: not implemented>\n");
            hasExited = true;
            break;
        case 17:
            result = sys_brk(read16(BX + 10), SP);
            if (!result) write16(BX + 18, brksize);
            break;
        case 18:
            result = sys_stat(str(read16(BX + 10)), read16(BX + 12));
            break;
        case 19:
        {
            off_t o = sys_lseek(read16(BX + 4), read32(BX + 10), read16(BX + 6));
            if (o == -1) {
                result = -1;
            } else {
                write32(BX + 10, o);
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
            result = sys_fstat(read16(BX + 4), read16(BX + 10));
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
            result = sys_access(str(read16(BX + 8)), read16(BX + 6));
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
            result = minix_signal();
            break;
        case 54:
            result = sys_ioctl(read16(BX + 4), read16(BX + 8), read16(BX + 18));
            break;
        case 55:
            fprintf(stderr, "<fcntl: not implemented>\n");
            hasExited = true;
            break;
        case 59:
            if (!minix_exec()) return true;
            break;
        case 60:
            result = sys_umask(read16(BX + 4));
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
            result = minix_sigaction();
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
    write16(BX + 2, result == -1 ? -errno : result);
    AX = 0;
    return true;
}

int VM::minix_fork() { // 2
    if (trace) fprintf(stderr, "<fork()>\n");
#ifdef NO_FORK
    VM vm = *this;
    vm.write16(BX + 2, 0);
    vm.AX = 0;
    vm.run();
    return vm.pid;
#else
    int result = fork();
    return result == -1 ? -1 : (result % 30000) + 1;
#endif
}

int VM::minix_exec() { // 59
    const char *path = str(read16(BX + 10));
    int frame = read16(BX + 12);
    int fsize = read16(BX + 6);
#if 0
    FILE *f = fopen("core", "wb");
    fwrite(data, 1, 0x10000, f);
    fclose(f);
#endif
    int argc = read16(frame);
    if (trace) {
        fprintf(stderr, "<exec(\"%s\"", path);
        for (int i = 2; i <= argc; i++) {
            fprintf(stderr, ", \"%s\"", data + frame + read16(frame + i * 2));
        }
        fprintf(stderr, ")>\n");
    }
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
    resetsig();
    start_sp = SP = 0x10000 - fsize;
    memcpy(data + start_sp, d + frame, fsize);
    if (d != t) delete[] d;
    int ad = start_sp + 2, p;
    for (int i = 0; i < 2; i++, ad += 2) {
        for (; (p = read16(ad)); ad += 2) {
            write16(ad, start_sp + p);
        }
    }
    return 0;
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

#include "OS.h"
#include <string.h>
#include <signal.h>

#define V6_SIGINT   2
#define V6_SIGINS   4
#define V6_SIGFPT   8
#define V6_SIGSEG  11

using namespace UnixV6;

OS::OS() {
}

OS::OS(const OS &os) : UnixBase(os) {
}

OS::~OS() {
}

void OS::disasm() {
    vm->disasm();
}

void OS::setstat(uint16_t addr, struct stat * st) {
    memset(vm->data + addr, 0, 36);
    vm->write16(addr, st->st_dev);
    vm->write16(addr + 2, st->st_ino);
    vm->write16(addr + 4, st->st_mode);
    vm->write8(addr + 6, st->st_nlink);
    vm->write8(addr + 7, st->st_uid);
    vm->write8(addr + 8, st->st_gid);
    vm->write8(addr + 9, st->st_size >> 16);
    vm->write16(addr + 10, st->st_size);
    vm->write16(addr + 28, st->st_atime >> 16);
    vm->write16(addr + 30, st->st_atime);
    vm->write16(addr + 32, st->st_mtime >> 16);
    vm->write16(addr + 34, st->st_mtime);
}

int OS::syscall(int *result, int n, int arg0, uint8_t *args) {
    *result = 0;
    switch (n) {
        case 0:
        {
            int p = read16(args);
            int nn = vm->read8(p);
            syscall(result, nn, arg0, vm->data + p + 2);
            return nn == 11/*exec*/ && !*result ? 0 : 2;
        }
        case 1:
            sys_exit((int16_t) arg0);
            return -1;
        case 2:
            *result = v6_fork();
            return 2;
        case 3:
            *result = sys_read(arg0, read16(args), read16(args + 2));
            return 4;
        case 4:
            *result = sys_write(arg0, read16(args), read16(args + 2));
            return 4;
        case 5:
            *result = sys_open(vm->str(read16(args)), read16(args + 2));
            return 4;
        case 6:
            *result = sys_close(arg0);
            return 0;
        case 7:
            *result = v6_wait();
            return 0;
        case 8:
            *result = sys_creat(vm->str(read16(args)), read16(args + 2));
            return 4;
        case 9:
            *result = sys_link(vm->str(read16(args)), vm->str(read16(args + 2)));
            return 4;
        case 10:
            *result = sys_unlink(vm->str(read16(args)));
            return 2;
        case 11:
            *result = v6_exec(vm->str(read16(args)), read16(args + 2));
            return *result ? 4 : 0;
        case 12:
            *result = sys_chdir(vm->str(read16(args)));
            return 2;
        case 13:
            fprintf(stderr, "<time: not implemented>\n");
            break;
        case 14:
            fprintf(stderr, "<mknod: not implemented>\n");
            break;
        case 15:
            *result = sys_chmod(vm->str(read16(args)), read16(args + 2));
            return 4;
        case 16:
            fprintf(stderr, "<chown: not implemented>\n");
            break;
        case 17:
            *result = v6_brk(read16(args));
            return 2;
        case 18:
            *result = sys_stat(vm->str(read16(args)), read16(args + 2));
            return 4;
        case 19:
            *result = v6_seek(arg0, read16(args), read16(args + 2));
            return 4;
        case 20:
            *result = sys_getpid();
            return 0;
        case 21:
            fprintf(stderr, "<mount: not implemented>\n");
            break;
        case 22:
            fprintf(stderr, "<umount: not implemented>\n");
            break;
        case 23:
            fprintf(stderr, "<setuid: not implemented>\n");
            break;
        case 24:
            fprintf(stderr, "<getuid: not implemented>\n");
            break;
        case 25:
            fprintf(stderr, "<stime: not implemented>\n");
            break;
        case 26:
            fprintf(stderr, "<ptrace: not implemented>\n");
            break;
        case 28:
            fprintf(stderr, "<fstat: not implemented>\n");
            break;
        case 31:
            fprintf(stderr, "<stty: not implemented>\n");
            break;
        case 32:
            fprintf(stderr, "<gtty: not implemented>\n");
            break;
        case 34:
            fprintf(stderr, "<nice: not implemented>\n");
            break;
        case 35:
            fprintf(stderr, "<sleep: not implemented>\n");
            break;
        case 36:
            fprintf(stderr, "<sync: not implemented>\n");
            break;
        case 37:
            fprintf(stderr, "<kill: not implemented>\n");
            break;
        case 38:
            fprintf(stderr, "<switch: not implemented>\n");
            break;
        case 41:
            *result = sys_dup(arg0);
            return 0;
        case 42:
            fprintf(stderr, "<pipe: not implemented>\n");
            break;
        case 43:
            fprintf(stderr, "<times: not implemented>\n");
            break;
        case 44:
            fprintf(stderr, "<prof: not implemented>\n");
            break;
        case 46:
            fprintf(stderr, "<setgid: not implemented>\n");
            break;
        case 47:
            fprintf(stderr, "<getgid: not implemented>\n");
            break;
        case 48:
            *result = v6_signal(read16(args), read16(args + 2));
            return 4;
        default:
            fprintf(stderr, "<%d: unknown syscall>\n", n);
            break;
    }
    sys_exit(-1);
    return -1;
}

int OS::v6_seek(int fd, off_t o, int w) { // 19
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

void OS::sighandler(int sig) {
    OS *cur = dynamic_cast<OS *> (current);
    if (cur) cur->sighandler2(sig);
}

int OS::convsig(int sig) {
    switch (sig) {
        case V6_SIGINT: return SIGINT;
        case V6_SIGINS: return SIGILL;
        case V6_SIGFPT: return SIGFPE;
        case V6_SIGSEG: return SIGSEGV;
    }
    return -1;
}

int OS::v6_signal(int sig, int h) {
    if (trace) fprintf(stderr, "<signal(%d, 0x%04x)>\n", sig, h);
    int s = convsig(sig);
    if (s < 0) {
        errno = EINVAL;
        return -1;
    }
    int oh = sighandlers[sig];
    sighandlers[sig] = h;
    setsig(s, h);
    return oh;
}

void OS::setsig(int sig, int h) {
    if (h == 0) {
        signal(sig, SIG_DFL);
    } else if (h & 1) {
        signal(sig, SIG_IGN);
    } else {
        signal(sig, &sighandler);
    }
}

void OS::resetsig() {
    for (int i = 0; i < nsig; i++) {
        uint16_t &sgh = sighandlers[i];
        if (sgh && !(sgh & 1)) {
            sighandlers[i] = 0;
            int s = convsig(i);
            if (s >= 0) signal(s, SIG_DFL);
        }
    }
}

void OS::swtch(bool reset) {
    for (int i = 0; i < nsig; i++) {
        int s = convsig(i);
        if (s >= 0) {
            if (reset) {
                signal(s, SIG_DFL);
            } else {
                setsig(s, sighandlers[i]);
            }
        }
    }
}

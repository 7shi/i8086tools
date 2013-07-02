#include "VM.h"
#include "disasm.h"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#ifdef WIN32
#include <windows.h>
#define NO_FORK
#else
#include <sys/wait.h>
#endif
#include <stack>
#include <map>
#include <algorithm>

#ifdef NO_FORK
static std::stack<std::pair<int, int> > exitcodes;
#endif

#ifdef WIN32
std::list<std::string> unlinks;

static void showError(int err) {
    fprintf(stderr, "%s", getErrorMessage(err).c_str());
}
#endif

int VM::close(int fd) {
    FileBase *f = file(fd);
    if (!f) return -1;

    files[fd] = NULL;
    if (--f->count) return 0;

    std::string path = f->path;
    delete f;

#ifdef WIN32
    std::list<std::string>::iterator it2 =
            std::find(unlinks.begin(), unlinks.end(), path);
    if (it2 != unlinks.end()) {
        if (trace)
            fprintf(stderr, "delayed unlink: %s\n", path.c_str());
        if (DeleteFileA(path.c_str()))
            unlinks.erase(it2);
        else if (trace)
            showError(GetLastError());
    }
#endif
    return 0;
}

static int convpid(int pid) {
#ifdef NO_FORK
    return ((getpid() << 4) % 30000) + pid;
#else
    return (getpid() % 30000) + 1;
#endif
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

void VM::minix_syscall() {
    int type = read16(BX + 2), result = 0;
    switch (type) {
        case 1:
            sys_exit((int16_t) read16(BX + 4));
            return;
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
            result = sys_brk(read16(BX + 10));
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
            result = sys_exec(str(read16(BX + 10)), read16(BX + 12), read16(BX + 6));
            if (!result) return;
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
            return;
    }
    write16(BX + 2, result == -1 ? -errno : result);
    AX = 0;
}

void VM::sys_exit(int code) {
    if (trace) fprintf(stderr, "<exit(%d)>\n", code);
    exitcode = code;
#ifdef NO_FORK
    exitcodes.push(std::pair<int, int>(convpid(pid), code));
#endif
    hasExited = true;
}

int VM::minix_fork() { // 2
    if (trace) fprintf(stderr, "<fork()>\n");
#ifdef NO_FORK
    VM vm = *this;
    vm.write16(BX + 2, 0);
    vm.AX = 0;
    vm.run();
    return convpid(vm.pid);
#else
    int result = fork();
    return result == -1 ? -1 : (result % 30000) + 1;
#endif
}

int VM::sys_read(int fd, int buf, int len) {
    if (trace) fprintf(stderr, "<read(%d, 0x%04x, %d)", fd, buf, len);
    int max = 0x10000 - buf;
    if (len > max) len = max;
    FileBase *f = file(fd);
    int result = f ? f->read(data + buf, len) : -1;
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VM::sys_write(int fd, int buf, int len) {
    if (trace) fprintf(stderr, "<write(%d, 0x%04x, %d)", fd, buf, len);
    int max = 0x10000 - buf;
    if (len > max) len = max;
    FileBase *f = file(fd);
    int result = -1;
    if (f) {
        if (trace && f->fd < 3) {
            fflush(stdout);
            fflush(stderr);
        }
        result = f->write(data + buf, len);
    }
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VM::sys_open(const char *path, int flag, mode_t mode) {
    if (flag & 64 /*O_CREAT*/) {
        if (trace) fprintf(stderr, "<open(\"%s\", %d, 0%03o)", path, flag, mode);
    } else {
        if (trace) fprintf(stderr, "<open(\"%s\", %d)", path, flag);
    }
    std::string path2 = convpath(path);
    int result = open(path2, flag, mode & ~umask);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VM::sys_close(int fd) {
    if (trace) fprintf(stderr, "<close(%d)", fd);
    int result = close(fd);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VM::sys_wait(int *status) {
    if (trace) fprintf(stderr, "<wait()");
#ifdef NO_FORK
    if (!exitcodes.empty()) {
        std::pair<int, int> ec = exitcodes.top();
        exitcodes.pop();
        *status = ec.second << 8;
        if (trace) fprintf(stderr, " => %d>\n", *status);
        return ec.first;
    } else {
        if (trace) fprintf(stderr, " => EINVAL>\n");
        errno = EINVAL;
        return -1;
    }
#else
    int result = wait(status);
    if (trace) fprintf(stderr, " => 0x%04x>\n", status);
#endif
}

int VM::sys_creat(const char *path, mode_t mode) {
    if (trace) fprintf(stderr, "<creat(\"%s\", 0%03o)", path, mode);
    std::string path2 = convpath(path);
#ifdef WIN32
    int result = open(path2, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, 0777);
#else
    int result = open(path2, O_CREAT | O_TRUNC | O_WRONLY, mode & ~umask);
#endif
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VM::sys_link(const char *src, const char *dst) {
    if (trace) fprintf(stderr, "<link(\"%s\", \"%s\")", src, dst);
    std::string src2 = convpath(src), dst2 = convpath(dst);
#ifdef WIN32
    int result = CopyFileA(src2.c_str(), dst2.c_str(), TRUE) ? 0 : -1;
    if (trace) fprintf(stderr, " => %d>\n", result);
    if (result) {
        errno = EINVAL;
        if (trace) showError(GetLastError());
    }
#else
    int result = link(src2.c_str(), dst2.c_str());
    if (trace) fprintf(stderr, " => %d>\n", result);
#endif
    return result;
}

int VM::sys_unlink(const char *path) {
    if (trace) fprintf(stderr, "<unlink(\"%s\")", path);
    std::string path2 = convpath(path);
#ifdef WIN32
    int result = DeleteFileA(path2.c_str()) ? 0 : -1;
    if (trace) fprintf(stderr, " => %d>\n", result);
    if (result) {
        errno = EINVAL;
        if (trace) showError(GetLastError());
        struct stat st;
        if (stat(path2.c_str(), &st) != -1) {
            if (trace) {
                fprintf(stderr, "<register delayed: %s>\n", path2.c_str());
            }
            unlinks.push_back(path2);
        }
    }
#else
    int result = unlink(path2.c_str());
    if (trace) fprintf(stderr, " => %d>\n", result);
#endif
    return result;
}

int VM::sys_time() {
    if (trace) fprintf(stderr, "<time()");
    int result = time(NULL);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VM::sys_chmod(const char *path, mode_t mode) {
    if (trace) fprintf(stderr, "<chmod(\"%s\", 0%03o)", path, mode);
    int result = chmod(convpath(path).c_str(), mode);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VM::sys_brk(int nd) {
    if (trace) fprintf(stderr, "<brk(0x%04x)", nd);
    if (nd < (int) dsize || nd >= ((SP - 0x400) & ~0x3ff)) {
        errno = ENOMEM;
        if (trace) fprintf(stderr, " => ENOMEM>\n");
        return -1;
    }
    brksize = nd;
    if (trace) fprintf(stderr, " => 0>\n");
    return 0;
}

int VM::sys_stat(const char *path, int p) {
    if (trace) fprintf(stderr, "<stat(\"%s\", 0x%04x)", path, p);
    struct stat st;
    int result;
    if (!(result = stat(path, &st))) {
        setstat(p, &st);
    }
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

off_t VM::sys_lseek(int fd, off_t o, int w) {
    if (trace) fprintf(stderr, "<lseek(%d, %ld, %d)", fd, o, w);
    FileBase *f = file(fd);
    off_t result = -1;
    if (f) result = f->lseek(o, w);
    if (trace) fprintf(stderr, " => %ld>\n", result);
    return result;
}

int VM::sys_getpid() {
    if (trace) fprintf(stderr, "<getpid()");
    int result = convpid(pid);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VM::sys_getuid() {
    if (trace) fprintf(stderr, "<getuid()");
#ifdef WIN32
    int result = 0;
#else
    int result = getuid();
#endif
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VM::sys_fstat(int fd, int p) {
    if (trace) fprintf(stderr, "<fstat(%d, 0x%04x)", fd, p);
    struct stat st;
    FileBase *f = file(fd);
    int result = -1;
    if (f) {
        if (f->fd <= 2) {
            errno = EBADF;
        } else if (!(result = fstat(f->fd, &st))) {
            setstat(p, &st);
        }
    }
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VM::sys_access(const char *path, mode_t mode) {
    if (trace) fprintf(stderr, "<access(\"%s\", 0%03o)", path, mode);
    std::string path2 = convpath(path);
    int result = access(path2.c_str(), mode);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VM::sys_getgid() {
    if (trace) fprintf(stderr, "<getgid()");
#ifdef WIN32
    int result = 0;
#else
    int result = getgid();
#endif
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VM::sys_ioctl(int fd, int rq, int d) {
    if (trace) fprintf(stderr, "<ioctl(%d, 0x%04x, 0x%04x)>\n", fd, rq, d);
    errno = EINVAL;
    return -1;
}

int VM::sys_exec(const char *path, int frame, int fsize) {
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

int VM::sys_umask(mode_t mask) {
    int result = umask;
    umask = mask;
    if (trace) fprintf(stderr, "<umask(0%03o) => 0%03o\n", umask, result);
    return result;
}

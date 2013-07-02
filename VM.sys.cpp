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
    return ((getpid() << 4) % 30000) +pid;
#else
    return getpid() % 30000;
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

VM::syshandler VM::syscalls[nsyscalls] = {
    { NULL, NULL}, //  0
    { "exit", &VM::_exit}, //  1
    { "fork", &VM::_fork}, //  2
    { "read", &VM::_read}, //  3
    { "write", &VM::_write}, //  4
    { "open", &VM::_open}, //  5
    { "close", &VM::_close}, //  6
    { "wait", &VM::_wait}, //  7
    { "creat", &VM::_creat}, //  8
    { "link", &VM::_link}, //  9
    { "unlink", &VM::_unlink}, // 10
    { "waitpid", NULL}, // 11
    { "chdir", NULL}, // 12
    { "time", &VM::_time}, // 13
    { "mknod", NULL}, // 14
    { "chmod", &VM::_chmod}, // 15
    { "chown", NULL}, // 16
    { "brk", &VM::_brk}, // 17
    { "stat", &VM::_stat}, // 18
    { "lseek", &VM::_lseek}, // 19
    { "getpid", &VM::_getpid}, // 20
    { "mount", NULL}, // 21
    { "umount", NULL}, // 22
    { "setuid", NULL}, // 23
    { "getuid", &VM::_getuid}, // 24
    { "stime", NULL}, // 25
    { "ptrace", NULL}, // 26
    { "alarm", NULL}, // 27
    { "fstat", &VM::_fstat}, // 28
    { "pause", NULL}, // 29
    { "utime", NULL}, // 30
    { NULL, NULL}, // 31
    { NULL, NULL}, // 32
    { "access", &VM::_access}, // 33
    { NULL, NULL}, // 34
    { NULL, NULL}, // 35
    { "sync", NULL}, // 36
    { "kill", NULL}, // 37
    { "rename", NULL}, // 38
    { "mkdir", NULL}, // 39
    { "rmdir", NULL}, // 40
    { "dup", NULL}, // 41
    { "pipe", NULL}, // 42
    { "times", NULL}, // 43
    { NULL, NULL}, // 44
    { NULL, NULL}, // 45
    { "setgid", NULL}, // 46
    { "getgid", &VM::_getgid}, // 47
    { "signal", &VM::_signal}, // 48
    { NULL, NULL}, // 49
    { NULL, NULL}, // 50
    { NULL, NULL}, // 51
    { NULL, NULL}, // 52
    { NULL, NULL}, // 53
    { "ioctl", &VM::_ioctl}, // 54
    { "fcntl", NULL}, // 55
    { NULL, NULL}, // 56
    { NULL, NULL}, // 57
    { NULL, NULL}, // 58
    { "exec", &VM::_exec}, // 59
    { "umask", &VM::_umask}, // 60
    { "chroot", NULL}, // 61
    { "setsid", NULL}, // 62
    { "getpgrp", NULL}, // 63
    { "ksig", NULL}, // 64
    { "unpause", NULL}, // 65
    { NULL, NULL}, // 66
    { "revive", NULL}, // 67
    { "task_reply", NULL}, // 68
    { NULL, NULL}, // 69
    { NULL, NULL}, // 70
    { "sigaction", &VM::_sigaction}, // 71
    { "sigsuspend", NULL}, // 72
    { "sigpending", NULL}, // 73
    { "sigprocmask", NULL}, // 74
    { "sigreturn", NULL}, // 75
    { "reboot", NULL}, // 76
    { "svrctl", NULL}, // 77
};

void VM::minix_syscall() {
    int type = read16(BX + 2);
    if (type < nsyscalls) {
        syshandler *sh = &syscalls[type];
        if (sh->name) {
            if (trace || !sh->f) {
                fprintf(stderr, "<%s", sh->name);
            }
            if (sh->f) {
                (this->*sh->f)();
                AX = 0;
            } else {
                fprintf(stderr, ": not implemented>\n");
                hasExited = true;
            }
            return;
        }
    }
    fprintf(stderr, "<%d: unknown syscall>\n", type);
    hasExited = true;
}

void VM::sys_exit(int code) {
    if (trace) fprintf(stderr, "(%d)>\n", code);
    exitcode = code;
#ifdef NO_FORK
    exitcodes.push(std::pair<int, int>(convpid(pid), code));
#endif
    hasExited = true;
}

void VM::_exit() { // 1
    sys_exit((int16_t) read16(BX + 4));
}

void VM::_fork() { // 2
    if (trace) fprintf(stderr, "()>\n");
#ifdef NO_FORK
    VM vm = *this;
    vm.write16(BX + 2, 0);
    vm.AX = 0;
    vm.run();
    write16(BX + 2, convpid(vm.pid));
#else
    int result = fork();
    write16(BX + 2, result == -1 ? -errno : result % 30000);
#endif
}

int VM::sys_read(int fd, int buf, int len) {
    if (trace) fprintf(stderr, "(%d, 0x%04x, %d)", fd, buf, len);
    int max = 0x10000 - buf;
    if (len > max) len = max;
    FileBase *f = file(fd);
    int result = f ? f->read(data + buf, len) : -1;
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

void VM::_read() { // 3
    int result = sys_read(read16(BX + 4), read16(BX + 10), read16(BX + 6));
    write16(BX + 2, result == -1 ? -errno : result);
}

int VM::sys_write(int fd, int buf, int len) {
    if (trace) fprintf(stderr, "(%d, 0x%04x, %d)", fd, buf, len);
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

void VM::_write() { // 4
    int result = sys_write(read16(BX + 4), read16(BX + 10), read16(BX + 6));
    write16(BX + 2, result == -1 ? -errno : result);
}

int VM::sys_open(const char *path, int flag, mode_t mode) {
    if (flag & 64 /*O_CREAT*/) {
        if (trace) fprintf(stderr, "(\"%s\", %d, 0%03o)", path, flag, mode & ~umask);
    } else {
        if (trace) fprintf(stderr, "(\"%s\", %d)", path, flag);
    }
    std::string path2 = convpath(path);
    int result = open(path2, flag, mode);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

void VM::_open() { // 5
    int flag = read16(BX + 6);
    int result;
    if (flag & 64 /*O_CREAT*/) {
        result = sys_open((const char *) (data + read16(BX + 10)), flag, read16(BX + 8));
    } else {
        result = sys_open((const char *) (data + read16(BX + 8)), flag);
    }
    write16(BX + 2, result == -1 ? -errno : result);
}

int VM::sys_close(int fd) {
    if (trace) fprintf(stderr, "(%d)", fd);
    int result = close(fd);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

void VM::_close() { // 6
    int result = sys_close(read16(BX + 4));
    write16(BX + 2, result == -1 ? -errno : result);
}

int VM::sys_wait(int *status) {
    if (trace) fprintf(stderr, "()");
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

void VM::_wait() { // 7
    int status, result = sys_wait(&status);
    write16(BX + 2, result == -1 ? -errno : result);
    write16(BX + 4, status);
}

int VM::sys_creat(const char *path, mode_t mode) {
    if (trace) fprintf(stderr, "(\"%s\", 0%03o)", path, mode);
    std::string path2 = convpath(path);
#ifdef WIN32
    int result = open(path2, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, 0777);
#else
    int result = open(path2, O_CREAT | O_TRUNC | O_WRONLY, mode & ~umask);
#endif
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

void VM::_creat() { // 8
    int result = sys_creat((const char *) (data + read16(BX + 8)), read16(BX + 6));
    write16(BX + 2, result == -1 ? -errno : result);
}

int VM::sys_link(const char *src, const char *dst) {
    if (trace) fprintf(stderr, "(\"%s\", \"%s\")", src, dst);
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

void VM::_link() { // 9
    int result = sys_link(
            (const char *) (data + read16(BX + 10)),
            (const char *) (data + read16(BX + 12)));
    write16(BX + 2, result == -1 ? -errno : result);
}

int VM::sys_unlink(const char *path) {
    if (trace) fprintf(stderr, "(\"%s\")", path);
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

void VM::_unlink() { // 10
    int result = sys_unlink((const char *) (data + read16(BX + 8)));
    write16(BX + 2, result == -1 ? -errno : result);
}

int VM::sys_time() {
    if (trace) fprintf(stderr, "()");
    int result = time(NULL);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

void VM::_time() { // 13
    int result = sys_time();
    write16(BX + 2, result == -1 ? -errno : 0);
    write32(BX + 10, result);
}

int VM::sys_chmod(const char *path, mode_t mode) {
    if (trace) fprintf(stderr, "(\"%s\", 0%03o)", path, mode);
    int result = chmod(convpath(path).c_str(), mode);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

void VM::_chmod() { // 15
    int result = sys_chmod((const char *) (data + read16(BX + 8)), read16(BX + 6));
    write16(BX + 2, result == -1 ? -errno : result);
}

int VM::sys_brk(int nd) {
    if (trace) fprintf(stderr, "(0x%04x)", nd);
    if (nd < (int) dsize || nd >= ((SP - 0x400) & ~0x3ff)) {
        errno = ENOMEM;
        if (trace) fprintf(stderr, " => ENOMEM>\n");
        return -1;
    }
    brksize = nd;
    if (trace) fprintf(stderr, " => 0>\n");
    return 0;
}

void VM::_brk() { // 17
    int result = sys_brk(read16(BX + 10));
    write16(BX + 2, result == -1 ? -errno : result);
    if (!result) write16(BX + 18, brksize);
}

int VM::sys_stat(const char *path, int p) {
    if (trace) fprintf(stderr, "(\"%s\", 0x%04x)", path, p);
    struct stat st;
    int result;
    if (!(result = stat(path, &st))) {
        setstat(p, &st);
    }
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

void VM::_stat() { // 18
    int result = sys_stat((const char *) (data + read16(BX + 10)), read16(BX + 12));
    write16(BX + 2, result == -1 ? -errno : result);
}

off_t VM::sys_lseek(int fd, off_t o, int w) {
    if (trace) fprintf(stderr, "(%d, %ld, %d)", fd, o, w);
    FileBase *f = file(fd);
    off_t result = -1;
    if (f) result = f->lseek(o, w);
    if (trace) fprintf(stderr, " => %ld>\n", result);
    return result;
}

void VM::_lseek() { // 19
    off_t result = sys_lseek(read16(BX + 4), read32(BX + 10), read16(BX + 6));
    write16(BX + 2, result == -1 ? -errno : 0);
    if (result != -1) write32(BX + 10, result);
}

int VM::sys_getpid() {
    if (trace) fprintf(stderr, "()");
    int result = convpid(pid);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

void VM::_getpid() { // 20
    int result = sys_getpid();
    write16(BX + 2, result == -1 ? -errno : result);
}

int VM::sys_getuid() {
    if (trace) fprintf(stderr, "()");
#ifdef WIN32
    int result = 0;
#else
    int result = getuid();
#endif
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

void VM::_getuid() { // 24
    int result = sys_getuid();
    write16(BX + 2, result == -1 ? -errno : result);
}

int VM::sys_fstat(int fd, int p) {
    if (trace) fprintf(stderr, "(%d, 0x%04x)", fd, p);
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

void VM::_fstat() { // 28
    int result = sys_fstat(read16(BX + 4), read16(BX + 10));
    write16(BX + 2, result == -1 ? -errno : result);
}

int VM::sys_access(const char *path, mode_t mode) {
    if (trace) fprintf(stderr, "(\"%s\", 0%03o)", path, mode);
    std::string path2 = convpath(path);
    int result = access(path2.c_str(), mode);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

void VM::_access() { // 33
    int result = sys_access((const char *) (data + read16(BX + 8)), read16(BX + 6));
    write16(BX + 2, result == -1 ? -errno : result);
}

int VM::sys_getgid() {
    if (trace) fprintf(stderr, "()");
#ifdef WIN32
    int result = 0;
#else
    int result = getgid();
#endif
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

void VM::_getgid() { // 47
    int result = sys_getgid();
    write16(BX + 2, result == -1 ? -errno : result);
}

int VM::sys_ioctl(int fd, int rq, int d) {
    if (trace) fprintf(stderr, "(%d, 0x%04x, 0x%04x)>\n", fd, rq, d);
    errno = EINVAL;
    return -1;
}

void VM::_ioctl() { // 54
    int result = sys_ioctl(read16(BX + 4), read16(BX + 8), read16(BX + 18));
    write16(BX + 2, result == -1 ? -errno : result);
}

int VM::sys_exec(const char *path, int frame, int fsize) {
#if 0
    FILE *f = fopen("core", "wb");
    fwrite(data, 1, 0x10000, f);
    fclose(f);
#endif
    int argc = read16(frame);
    if (trace) {
        fprintf(stderr, "(\"%s\"", path);
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

void VM::_exec() { // 59
    int result = sys_exec((const char *) (data + read16(BX + 10)), read16(BX + 12), read16(BX + 6));
    if (result) write16(BX + 2, -errno);
}

int VM::sys_umask(mode_t mask) {
    int result = umask;
    umask = mask;
    if (trace) fprintf(stderr, "(0%03o) => 0%03o\n", umask, result);
    return result;
}

void VM::_umask() { // 60
    int result = sys_umask(read16(BX + 4));
    write16(BX + 2, result);
}

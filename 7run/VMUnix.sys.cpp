#include "VMUnix.h"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#ifdef WIN32
#include <windows.h>
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

int VMUnix::close(int fd) {
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

void VMUnix::sys_exit(int code) {
    if (trace) fprintf(stderr, "<exit(%d)>\n", code);
    exitcode = code;
#ifdef NO_FORK
    exitcodes.push(std::pair<int, int>(pid, code));
#endif
    hasExited = true;
}

int VMUnix::sys_read(int fd, int buf, int len) {
    if (trace) fprintf(stderr, "<read(%d, 0x%04x, %d)", fd, buf, len);
    int max = 0x10000 - buf;
    if (len > max) len = max;
    FileBase *f = file(fd);
    int result = f ? f->read(data + buf, len) : -1;
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VMUnix::sys_write(int fd, int buf, int len) {
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

int VMUnix::sys_open(const char *path, int flag, mode_t mode) {
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

int VMUnix::sys_close(int fd) {
    if (trace) fprintf(stderr, "<close(%d)", fd);
    int result = close(fd);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VMUnix::sys_wait(int *status) {
#ifdef NO_FORK
    if (exitcodes.empty()) {
        if (trace) fprintf(stderr, "<wait() => EINVAL>\n");
        errno = EINVAL;
        return -1;
    }
    std::pair<int, int> ec = exitcodes.top();
    exitcodes.pop();
    *status = (ec.second << 8) | 14;
    if (trace) fprintf(stderr, "<wait() => %d>\n", *status);
    return ec.first;
#else
    int result = wait(status);
    if (trace) fprintf(stderr, "<wait() => 0x%04x>\n", *status);
    return result;
#endif
}

int VMUnix::sys_creat(const char *path, mode_t mode) {
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

int VMUnix::sys_link(const char *src, const char *dst) {
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

int VMUnix::sys_unlink(const char *path) {
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

int VMUnix::sys_chdir(const char *path) {
    if (trace) fprintf(stderr, "<chdir(\"%s\")", path);
    std::string path2 = convpath(path);
    int result = chdir(path2.c_str());
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VMUnix::sys_time() {
    if (trace) fprintf(stderr, "<time()");
    int result = time(NULL);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VMUnix::sys_chmod(const char *path, mode_t mode) {
    if (trace) fprintf(stderr, "<chmod(\"%s\", 0%03o)", path, mode);
    int result = chmod(convpath(path).c_str(), mode);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VMUnix::sys_brk(int nd, int sp) {
    if (trace) fprintf(stderr, "<brk(0x%04x)", nd);
    if (nd < (int) dsize || nd >= ((sp - 0x400) & ~0x3ff)) {
        errno = ENOMEM;
        if (trace) fprintf(stderr, " => ENOMEM>\n");
        return -1;
    }
    brksize = nd;
    if (trace) fprintf(stderr, " => 0>\n");
    return 0;
}

int VMUnix::sys_stat(const char *path, int p) {
    if (trace) fprintf(stderr, "<stat(\"%s\", 0x%04x)", path, p);
    struct stat st;
    int result;
    if (!(result = stat(path, &st))) {
        setstat(p, &st);
    }
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

off_t VMUnix::sys_lseek(int fd, off_t o, int w) {
    if (trace) fprintf(stderr, "<lseek(%d, %ld, %d)", fd, o, w);
    FileBase *f = file(fd);
    off_t result = -1;
    if (f) result = f->lseek(o, w);
    if (trace) fprintf(stderr, " => %ld>\n", result);
    return result;
}

int VMUnix::sys_getpid() {
    if (trace) fprintf(stderr, "<getpid()");
    int result = pid;
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VMUnix::sys_getuid() {
    if (trace) fprintf(stderr, "<getuid()");
#ifdef WIN32
    int result = 0;
#else
    int result = getuid();
#endif
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VMUnix::sys_fstat(int fd, int p) {
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

int VMUnix::sys_access(const char *path, mode_t mode) {
    if (trace) fprintf(stderr, "<access(\"%s\", 0%03o)", path, mode);
    std::string path2 = convpath(path);
    int result = access(path2.c_str(), mode);
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VMUnix::sys_getgid() {
    if (trace) fprintf(stderr, "<getgid()");
#ifdef WIN32
    int result = 0;
#else
    int result = getgid();
#endif
    if (trace) fprintf(stderr, " => %d>\n", result);
    return result;
}

int VMUnix::sys_ioctl(int fd, int rq, int d) {
    if (trace) fprintf(stderr, "<ioctl(%d, 0x%04x, 0x%04x)>\n", fd, rq, d);
    errno = EINVAL;
    return -1;
}

int VMUnix::sys_umask(mode_t mask) {
    int result = umask;
    umask = mask;
    if (trace) fprintf(stderr, "<umask(0%03o) => 0%03o\n", umask, result);
    return result;
}

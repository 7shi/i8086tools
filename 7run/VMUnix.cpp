#include "VMUnix.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

VMUnix *VMUnix::current;

static int createpid() {
#ifdef NO_FORK
    static int pid_max;
    return ((getpid() << 4) % 30000) + (++pid_max);
#else
    return (getpid() % 30000) + 1;
#endif
}

VMUnix::VMUnix() : umask(0) {
    pid = createpid();
    files.push_back(new File(0, "stdin"));
    files.push_back(new File(1, "stdout"));
    files.push_back(new File(2, "stderr"));
}

VMUnix::VMUnix(const VMUnix &vm) {
    pid = createpid();
    umask = vm.umask;
    files = vm.files;
    for (int i = 0; i < (int) files.size(); i++) {
        FileBase *f = files[i];
        if (f) ++f->count;
    }
}

VMUnix::~VMUnix() {
    for (int i = 0; i <= (int) files.size(); i++) {
        close(i);
    }
}

bool VMUnix::load(const std::string &fn) {
    std::string fn2 = convpath(fn);
    const char *file = fn2.c_str();
    FILE *f = fopen(file, "rb");
    if (!f) {
        fprintf(stderr, "can not open: %s\n", file);
        return false;
    }
    struct stat st;
    fstat(fileno(f), &st);
    bool ret = load2(fn, f, st.st_size);
    fclose(f);
    return ret;
}

void VMUnix::run(
        const std::vector<std::string> &args,
        const std::vector<std::string> &envs) {
    if (trace >= 2) vmbase->showHeader();
    setArgs(args, envs);
    run();
}

void VMUnix::run() {
    VMUnix *from = current;
    swtch(this);
    vmbase->hasExited = false;
    vmbase->run2();
    swtch(from);
}

int VMUnix::getfd() {
    int len = files.size();
    for (int i = 0; i < len; i++) {
        if (!files[i]) return i;
    }
    files.push_back(NULL);
    return len;
}

int VMUnix::open(const std::string &path, int flag, int mode) {
#ifdef WIN32
    flag |= O_BINARY;
#endif
    File *f = new File(path, flag, mode);
    if (f->fd == -1) {
        delete f;
        return -1;
    }
    int fd = getfd();
    files[fd] = f;
    return fd;
}

int VMUnix::dup(int fd) {
    FileBase *f = file(fd);
    if (!f) return -1;

    FileBase *f2 = f->dup();
    int fd2 = getfd();
    files[fd2] = f2;
    return fd2;
}

FileBase *VMUnix::file(int fd) {
    if (fd < 0 || fd >= (int) files.size() || !files[fd]) {
        errno = EBADF;
        return NULL;
    }
    return files[fd];
}

void VMUnix::swtch(VMUnix *to) {
    if (to) to->swtch();
    else swtch(true);
    current = to;
}

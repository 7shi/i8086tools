#include "VMUnix.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int trace;
int exitcode;
VMUnix *VMUnix::current;

void VMUnix::init() {
    text = new uint8_t[0x10000];
    hasExited = false;
#ifdef NO_FORK
    static int pid_max;
    pid = ((getpid() << 4) % 30000) + (++pid_max);
#else
    pid = (getpid() % 30000) + 1;
#endif
}

VMUnix::VMUnix() : data(NULL), tsize(0), umask(0) {
    init();
    memset(text, 0, 0x10000);
    files.push_back(new File(0, "stdin"));
    files.push_back(new File(1, "stdout"));
    files.push_back(new File(2, "stderr"));
}

VMUnix::VMUnix(const VMUnix &vm) {
    init();
    memcpy(text, vm.text, 0x10000);
    if (vm.data == vm.text) {
        data = text;
    } else {
        data = new uint8_t[0x10000];
        memcpy(data, vm.data, 0x10000);
    }
    tsize = vm.tsize;
    dsize = vm.dsize;
    umask = vm.umask;
    files = vm.files;
    for (int i = 0; i < (int) files.size(); i++) {
        FileBase *f = files[i];
        if (f) ++f->count;
    }
}

VMUnix::~VMUnix() {
    if (data != text) delete[] data;
    delete[] text;
    for (int i = 0; i <= (int) files.size(); i++) {
        close(i);
    }
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

FileBase *VMUnix::file(int fd) {
    if (fd < 0 || fd >= (int) files.size() || !files[fd]) {
        errno = EBADF;
        return NULL;
    }
    return files[fd];
}

void VMUnix::swtch(VMUnix *to) {
    if (to) to->swtch(); else swtch(true);
    current = to;
}

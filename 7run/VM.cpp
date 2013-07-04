#include "VM.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int trace;
int exitcode;
static int pid_max;
VM *VM::current;

void VM::init() {
    text = new uint8_t[0x10000];
    hasExited = false;
#ifdef NO_FORK
    pid = ((getpid() << 4) % 30000) + (++pid_max);
#else
    pid = (getpid() % 30000) + 1;
#endif
}

VM::VM() : data(NULL), tsize(0), umask(0) {
    init();
    memset(text, 0, 0x10000);
    memset(sigacts, 0, sizeof (sigacts));
    files.push_back(new File(0, "stdin"));
    files.push_back(new File(1, "stdout"));
    files.push_back(new File(2, "stderr"));
}

VM::VM(const VM &vm) {
    init();
    memcpy(text, vm.text, 0x10000);
    memcpy(sigacts, vm.sigacts, sizeof(sigacts));
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

VM::~VM() {
    if (data != text) delete[] data;
    delete[] text;
    for (int i = 0; i <= (int) files.size(); i++) {
        close(i);
    }
}

int VM::getfd() {
    int len = files.size();
    for (int i = 0; i < len; i++) {
        if (!files[i]) return i;
    }
    files.push_back(NULL);
    return len;
}

int VM::open(const std::string &path, int flag, int mode) {
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

FileBase *VM::file(int fd) {
    if (fd < 0 || fd >= (int) files.size() || !files[fd]) {
        errno = EBADF;
        return NULL;
    }
    return files[fd];
}

void VM::swtch(VM *to) {
    for (int i = 0; i < nsig; i++) {
        int s = convsig(i);
        if (s >= 0) {
            if (!to) {
                signal(s, SIG_DFL);
            } else {
                setsig(s, to->sigacts[i].handler);
            }
        }
    }
    current = to;
}
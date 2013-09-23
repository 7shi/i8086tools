#include "OS.h"
#include <string.h>
#include <signal.h>

#define V6_SIGINT   2
#define V6_SIGINS   4
#define V6_SIGFPT   8
#define V6_SIGSEG  11

using namespace UnixV6;

OS::OS() {
    memset(sighandlers, 0, sizeof (sighandlers));
}

OS::OS(const OS &os) : UnixBase(os) {
    memset(sighandlers, 0, sizeof (sighandlers));
}

OS::~OS() {
}

void OS::readsym(FILE *f, int ssize) {
    if (!ssize) return;

    uint8_t buf[12];
    for (int i = 0; i < ssize; i += 12) {
        fread(buf, sizeof (buf), 1, f);
        Symbol sym = {
            readstr(buf, 8), ::read16(buf + 8), ::read16(buf + 10)
        };
        int t = sym.type;
        if (t < 6) {
            t = "uatdbc"[t];
        } else if (0x20 <= t && t < 0x26) {
            t = "UATDBC"[t - 0x20];
        }
        switch (t) {
            case 't':
            case 'T':
                if (!sym.name.empty() && !startsWith(sym.name, "~")) {
                    vm->syms[1][sym.addr] = sym;
                }
                break;
            case 0x1f:
                vm->syms[0][sym.addr] = sym;
                break;
        }
    }
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

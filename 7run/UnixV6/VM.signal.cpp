#include "VM.h"
#include "../PDP11/regs.h"
#include <string.h>
#include <signal.h>

#define V6_SIGINT   2
#define V6_SIGINS   4
#define V6_SIGFPT   8
#define V6_SIGSEG  11

using namespace UnixV6;

void VM::sighandler(int sig) {
    VM *cur = dynamic_cast<VM *> (current);
    if (cur) cur->sighandler2(sig);
}

void VM::sighandler2(int sig) {
    uint16_t r[8];
    memcpy(r, this->r, sizeof (r));
    bool Z = this->Z, N = this->N, C = this->C, V = this->V;
    write16((this->SP -= 2), PC);
    this->PC = sighandlers[sig];
    while (!hasExited && !(this->PC == PC && this->SP == SP)) {
        run1();
    }
    if (!hasExited) {
        memcpy(this->r, r, sizeof (r));
        this->Z = Z;
        this->N = N;
        this->C = C;
        this->V = V;
    }
}

int VM::convsig(int sig) {
    switch (sig) {
        case V6_SIGINT: return SIGINT;
        case V6_SIGINS: return SIGILL;
        case V6_SIGFPT: return SIGFPE;
        case V6_SIGSEG: return SIGSEGV;
    }
    return -1;
}

int VM::v6_signal(int sig, int h) {
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

void VM::setsig(int sig, int h) {
    if (h == 0) {
        signal(sig, SIG_DFL);
    } else if (h & 1) {
        signal(sig, SIG_IGN);
    } else {
        signal(sig, &sighandler);
    }
}

void VM::resetsig() {
    for (int i = 0; i < nsig; i++) {
        uint16_t &sgh = sighandlers[i];
        if (sgh && !(sgh & 1)) {
            sighandlers[i] = 0;
            int s = convsig(i);
            if (s >= 0) signal(s, SIG_DFL);
        }
    }
}

void VM::swtch(bool reset) {
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

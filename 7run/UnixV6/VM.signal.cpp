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
    memcpy(r, cpu.r, sizeof (r));
    bool Z = cpu.Z, N = cpu.N, C = cpu.C, V = cpu.V;
    cpu.write16((cpu.SP -= 2), cpu.PC);
    cpu.PC = sighandlers[sig];
    while (!cpu.hasExited && !(cpu.PC == PC && cpu.SP == SP)) {
        cpu.run1();
    }
    if (!cpu.hasExited) {
        memcpy(cpu.r, r, sizeof (r));
        cpu.Z = Z;
        cpu.N = N;
        cpu.C = C;
        cpu.V = V;
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

#include "VM.h"
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#define MX_SIGINT   2
#define MX_SIGILL   4
#define MX_SIGFPE   8
#define MX_SIGSEGV 11

#define MX_SIG_DFL  0
#define MX_SIG_IGN  1

static int convsig(int sig) {
    switch (sig) {
        case MX_SIGINT: return SIGINT;
        case MX_SIGILL: return SIGILL;
        case MX_SIGFPE: return SIGFPE;
        case MX_SIGSEGV: return SIGSEGV;
    }
    return -1;
}

void VM::sighandler(int sig) {
    current->ip = current->sigacts[sig].handler;
}

void VM::_signal() { // 48
    int sig = read16(BX + 4);
    int sgh = read16(BX + 14);
    if (trace) fprintf(stderr, "(%d, 0x%04x)\n", sig, sgh);
    write16(BX + 2, -EINVAL);
}

void VM::_sigaction() { // 71
    int sig = read16(BX + 6);
    int act = read16(BX + 10);
    int oact = read16(BX + 12);
    if (trace) fprintf(stderr, "(%d, 0x%04x, 0x%04x)\n", sig, act, oact);
    int s = convsig(sig);
    if (s < 0) {
        write16(BX + 2, -EINVAL);
        return;
    }
    write16(BX + 2, 0);
    write16(oact, sigacts[sig].handler);
    write16(oact + 2, sigacts[sig].mask);
    write16(oact + 4, sigacts[sig].flags);
    sigact sa = {read16(act), read16(act + 2), read16(act + 4)};
    switch (sa.handler) {
        case MX_SIG_DFL: signal(s, SIG_DFL);
            break;
        case MX_SIG_IGN: signal(s, SIG_IGN);
            break;
        default: signal(s, &sighandler);
            break;
    }
    sigacts[sig] = sa;
}

void VM::swtch(VM *to) {
    for (int i = 0; i < nsig; i++) {
        int s = convsig(i);
        if (s >= 0) {
            if (!to) {
                signal(s, SIG_DFL);
            } else {
                switch (to->sigacts[i].handler) {
                    case MX_SIG_DFL: signal(s, SIG_DFL);
                        break;
                    case MX_SIG_IGN: signal(s, SIG_IGN);
                        break;
                    default: signal(s, &sighandler);
                        break;
                }
            }
        }
    }
    current = to;
}

void VM::resetsig() {
    for (int i = 0; i < nsig; i++) {
        switch (sigacts[i].handler) {
            case MX_SIG_DFL:
            case MX_SIG_IGN:
                break;
            default:
                sigacts[i].handler = MX_SIG_DFL;
                int s = convsig(i);
                if (s >= 0) signal(s, SIG_DFL);
                break;
        }
    }
}

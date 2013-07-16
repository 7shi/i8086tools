#include "OS.h"
#include "../i8086/regs.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#define MX_SIGINT   2
#define MX_SIGILL   4
#define MX_SIGFPE   8
#define MX_SIGSEGV 11

#define MX_SIG_DFL  0
#define MX_SIG_IGN  1

using namespace Minix2;

void OS::sighandler(int sig) {
    OS *cur = dynamic_cast<OS *> (current);
    if (cur) cur->sighandler2(sig);
}

void OS::sighandler2(int sig) {
    uint16_t ip = cpu.ip, r[8];
    memcpy(r, cpu.r, sizeof (r));
    bool OF = cpu.OF, DF = cpu.DF, SF = cpu.SF;
    bool ZF = cpu.ZF, PF = cpu.PF, CF = cpu.CF;
    cpu.write16((cpu.SP -= 2), ip);
    cpu.ip = sigacts[sig].handler;
    while (!cpu.hasExited && !(cpu.ip == ip && cpu.SP == SP)) {
        cpu.run1();
    }
    if (!cpu.hasExited) {
        memcpy(cpu.r, r, sizeof (r));
        cpu.OF = OF;
        cpu.DF = DF;
        cpu.SF = SF;
        cpu.ZF = ZF;
        cpu.PF = PF;
        cpu.CF = CF;
    }
}

int OS::convsig(int sig) {
    switch (sig) {
        case MX_SIGINT: return SIGINT;
        case MX_SIGILL: return SIGILL;
        case MX_SIGFPE: return SIGFPE;
        case MX_SIGSEGV: return SIGSEGV;
    }
    return -1;
}

int OS::minix_signal() { // 48
    int sig = cpu.read16(cpu.BX + 4);
    int sgh = cpu.read16(cpu.BX + 14);
    if (trace) fprintf(stderr, "<signal(%d, 0x%04x)>\n", sig, sgh);
    int s = convsig(sig);
    if (s < 0) {
        errno = EINVAL;
        return -1;
    }
    int oh = sigacts[sig].handler;
    sigacts[sig].handler = sgh;
    sigacts[sig].mask = sigacts[sig].flags = 0;
    setsig(s, sgh);
    return oh;
}

int OS::minix_sigaction() { // 71
    int sig = cpu.read16(cpu.BX + 6);
    int act = cpu.read16(cpu.BX + 10);
    int oact = cpu.read16(cpu.BX + 12);
    if (trace) fprintf(stderr, "<sigaction(%d, 0x%04x, 0x%04x)>\n", sig, act, oact);
    int s = convsig(sig);
    if (s < 0) {
        errno = EINVAL;
        return -1;
    }
    cpu.write16(oact, sigacts[sig].handler);
    cpu.write16(oact + 2, sigacts[sig].mask);
    cpu.write16(oact + 4, sigacts[sig].flags);
    sigact sa = {cpu.read16(act), cpu.read16(act + 2), cpu.read16(act + 4)};
    sigacts[sig] = sa;
    setsig(s, sa.handler);
    return 0;
}

void OS::setsig(int sig, int h) {
    switch (h) {
        case MX_SIG_DFL: signal(sig, SIG_DFL);
            break;
        case MX_SIG_IGN: signal(sig, SIG_IGN);
            break;
        default: signal(sig, &sighandler);
            break;
    }
}

void OS::resetsig() {
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

void OS::swtch(bool reset) {
    for (int i = 0; i < nsig; i++) {
        int s = convsig(i);
        if (s >= 0) {
            if (reset) {
                signal(s, SIG_DFL);
            } else {
                setsig(s, sigacts[i].handler);
            }
        }
    }
}

#include "VMMinix2.h"
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#define MX_SIGINT   2
#define MX_SIGILL   4
#define MX_SIGFPE   8
#define MX_SIGSEGV 11

#define MX_SIG_DFL  0
#define MX_SIG_IGN  1

void VMMinix2::sighandler(int sig) {
    VMMinix2 *cur = dynamic_cast<VMMinix2 *> (current);
    if (cur) cur->sighandler2(sig);
}

void VMMinix2::sighandler2(int sig) {
    ip = sigacts[sig].handler;
}

int VMMinix2::convsig(int sig) {
    switch (sig) {
        case MX_SIGINT: return SIGINT;
        case MX_SIGILL: return SIGILL;
        case MX_SIGFPE: return SIGFPE;
        case MX_SIGSEGV: return SIGSEGV;
    }
    return -1;
}

int VMMinix2::minix_signal() { // 48
    int sig = read16(BX + 4);
    int sgh = read16(BX + 14);
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

int VMMinix2::minix_sigaction() { // 71
    int sig = read16(BX + 6);
    int act = read16(BX + 10);
    int oact = read16(BX + 12);
    if (trace) fprintf(stderr, "<sigaction(%d, 0x%04x, 0x%04x)>\n", sig, act, oact);
    int s = convsig(sig);
    if (s < 0) {
        errno = EINVAL;
        return -1;
    }
    write16(oact, sigacts[sig].handler);
    write16(oact + 2, sigacts[sig].mask);
    write16(oact + 4, sigacts[sig].flags);
    sigact sa = {read16(act), read16(act + 2), read16(act + 4)};
    sigacts[sig] = sa;
    setsig(s, sa.handler);
    return 0;
}

void VMMinix2::setsig(int sig, int h) {
    switch (h) {
        case MX_SIG_DFL: signal(sig, SIG_DFL);
            break;
        case MX_SIG_IGN: signal(sig, SIG_IGN);
            break;
        default: signal(sig, &sighandler);
            break;
    }
}

void VMMinix2::resetsig() {
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

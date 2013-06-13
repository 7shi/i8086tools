#include "VM.h"
#include <stdio.h>
#include <signal.h>

#define MX_SIGINT   2
#define MX_SIGILL   4
#define MX_SIGFPE   8
#define MX_SIGSEGV 11

#define MX_SIG_DFL  0
#define MX_SIG_IGN  1

void VM::sighandler(int sig) {
	current->ip = current->sigacts[sig].sa_handler;
}

void VM::_signal() { // 48
	int sig = read16(BX + 4);
	int sgh = read16(BX + 14);
	if (trace) fprintf(stderr, "(%d, 0x%04x)\n", sig, sgh);
	switch (sig) {
	case MX_SIGINT : sig = SIGINT ; break;
	case MX_SIGILL : sig = SIGILL ; break;
	case MX_SIGFPE : sig = SIGFPE ; break;
	case MX_SIGSEGV: sig = SIGSEGV; break;
	default:
		write16(BX + 2, -EINVAL);
		return;
	}
	write16(BX + 2, sigacts[sig].sa_handler);
	sigacts[sig].sa_handler = sgh;
	sigacts[sig].sa_mask    = 0;
	sigacts[sig].sa_flags   = 0;
	switch (sgh) {
	case MX_SIG_DFL: signal(sig, SIG_DFL); break;
	case MX_SIG_IGN: signal(sig, SIG_IGN); break;
	default: signal(sig, &sighandler); break;
	}
}

void VM::_sigaction() { // 71
	int sig  = read16(BX + 6);
	int act  = read16(BX + 10);
	int oact = read16(BX + 12);
	if (trace) fprintf(stderr, "(%d, 0x%04x, 0x%04x)\n", sig, act, oact);
	switch (sig) {
	case MX_SIGINT : sig = SIGINT ; break;
	case MX_SIGILL : sig = SIGILL ; break;
	case MX_SIGFPE : sig = SIGFPE ; break;
	case MX_SIGSEGV: sig = SIGSEGV; break;
	default:
		write16(BX + 2, -EINVAL);
		return;
	}
	*(sigact *)(data + oact) = sigacts[sig];
	sigacts[sig] = *(sigact *)(data + act);
	switch (sigacts[sig].sa_handler) {
	case MX_SIG_DFL: signal(sig, SIG_DFL); break;
	case MX_SIG_IGN: signal(sig, SIG_IGN); break;
	default: signal(sig, &sighandler); break;
	}
	write16(BX + 2, 0);
}

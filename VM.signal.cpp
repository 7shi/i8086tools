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

void VM::_sigaction() { // 71
	int sig  = read16(BX + 6);
	int act  = read16(BX + 10);
	int oact = read16(BX + 12);
	if (trace) fprintf(stderr, "(%d, %04x, %04x)\n", sig, act, oact);
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

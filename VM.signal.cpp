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
	write16(BX + 2, -EINVAL);
}

void VM::_sigaction() { // 71
	int sig  = read16(BX + 6);
	int act  = read16(BX + 10);
	int oact = read16(BX + 12);
	if (trace) fprintf(stderr, "(%d, 0x%04x, 0x%04x)\n", sig, act, oact);
	int s;
	switch (sig) {
	case MX_SIGINT : s = SIGINT ; break;
	case MX_SIGILL : s = SIGILL ; break;
	case MX_SIGFPE : s = SIGFPE ; break;
	case MX_SIGSEGV: s = SIGSEGV; break;
	default:
		write16(BX + 2, -EINVAL);
		return;
	}
	write16(BX + 2, 0);
	write16(oact    , sigacts[sig].sa_handler);
	write16(oact + 2, sigacts[sig].sa_mask   );
	write16(oact + 4, sigacts[sig].sa_flags  );
	sigact sa = { read16(act), read16(act + 2), read16(act + 4) };
	switch (sa.sa_handler) {
	case MX_SIG_DFL: signal(s, SIG_DFL); break;
	case MX_SIG_IGN: signal(s, SIG_IGN); break;
	default: signal(s, &sighandler); break;
	}
	sigacts[sig] = sa;
}

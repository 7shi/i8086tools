#include "VM.h"
#include "disasm.h"
#include <stdio.h>
#include <unistd.h>

bool VM::minix_syscall() {
	int type = read16(BX + 2);
	switch (type) {
	case 1: // exit
		exit_status = read16(BX + 4);
		return false;
	case 4: // write
		type = write(read16(BX + 4), data + read16(BX + 10), read16(BX + 6));
		break;
	default:
		if (!verbose) {
			fprintf(stderr, header);
			debug(disasm1(text + ip - 2, ip - 2, tsize));
		}
		fprintf(stderr, "unknown system call: %d\n", type);
		return false;
	}
	AX = 0;
	write16(BX + 2, type);
	return true;
}

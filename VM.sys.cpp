#include "VM.h"
#include "disasm.h"
#include <stdio.h>
#include <unistd.h>

VM::syshandler VM::syscalls[nsyscalls] = {
	{ NULL     , NULL          }, //  0
	{ "exit"   , &VM::_exit    }, //  1
	{ "fork"   , NULL          }, //  2
	{ "read"   , NULL          }, //  3
	{ "write"  , &VM::_write   }, //  4
	{ "open"   , NULL          }, //  5
	{ "close"  , NULL          }, //  6
	{ "wait"   , NULL          }, //  7
	{ "creat"  , NULL          }, //  8
	{ "link"   , NULL          }, //  9
	{ "unlink" , NULL          }, // 10
	{ "waitpid", NULL          }, // 11
	{ "chdir"  , NULL          }, // 12
	{ "time"   , NULL          }, // 13
	{ "mknod"  , NULL          }, // 14
	{ "chmod"  , NULL          }, // 15
	{ "chown"  , NULL          }, // 16
	{ "brk"    , NULL          }, // 17
	{ "stat"   , NULL          }, // 18
	{ "lseek"  , NULL          }, // 19
	{ "getpid" , NULL          }, // 20
	{ "mount"  , NULL          }, // 21
	{ "umount" , NULL          }, // 22
	{ "setuid" , NULL          }, // 23
	{ "getuid" , NULL          }, // 24
	{ "stime"  , NULL          }, // 25
	{ "ptrace" , NULL          }, // 26
	{ "alarm"  , NULL          }, // 27
	{ "fstat"  , NULL          }, // 28
	{ "pause"  , NULL          }, // 29
	{ "utime"  , NULL          }, // 30
	{ NULL     , NULL          }, // 31
	{ NULL     , NULL          }, // 32
	{ "access" , NULL          }, // 33
	{ NULL     , NULL          }, // 34
	{ NULL     , NULL          }, // 35
	{ "sync"   , NULL          }, // 36
	{ "kill"   , NULL          }, // 37
	{ "rename" , NULL          }, // 38
	{ "mkdir"  , NULL          }, // 39
	{ "rmdir"  , NULL          }, // 40
	{ "dup"    , NULL          }, // 41
	{ "pipe"   , NULL          }, // 42
	{ "times"  , NULL          }, // 43
	{ NULL     , NULL          }, // 44
	{ NULL     , NULL          }, // 45
	{ "setgid" , NULL          }, // 46
	{ "getgid" , NULL          }, // 47
	{ "signal" , NULL          }, // 48
	{ NULL     , NULL          }, // 49
	{ NULL     , NULL          }, // 50
	{ NULL     , NULL          }, // 51
	{ NULL     , NULL          }, // 52
	{ NULL     , NULL          }, // 53
	{ "ioctl"  , NULL          }, // 54
	{ "fcntl"  , NULL          }, // 55
	{ NULL     , NULL          }, // 56
	{ NULL     , NULL          }, // 57
	{ NULL     , NULL          }, // 58
	{ "exec"   , NULL          }, // 59
	{ "umask"  , NULL          }, // 60
	{ "chroot" , NULL          }, // 61
	{ "setsid" , NULL          }, // 62
	{ "getpgrp", NULL          }, // 63
};

void VM::minix_syscall() {
	int type = read16(BX + 2);
	if (type < nsyscalls && syscalls[type].f) {
		(this->*syscalls[type].f)();
		return;
	}
	if (!verbose) {
		fprintf(stderr, header);
		debug(disasm1(text + ip - 2, ip - 2, tsize));
	}
	if (type < nsyscalls && syscalls[type].name) {
		fprintf(stderr, "not implemented: %s\n", syscalls[type].name);
	} else {
		fprintf(stderr, "unknown system call: %d\n", type);
	}
	hasExited = true;
}

void VM::_exit() {
	exit_status = read16(BX + 4);
	hasExited = true;
}

void VM::_write() {
	int result = write(read16(BX + 4), data + read16(BX + 10), read16(BX + 6));
	AX = 0;
	write16(BX + 2, result);
}

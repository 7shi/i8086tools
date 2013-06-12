#include "VM.h"
#include "disasm.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>

std::map<int, std::string> fd2name;

VM::syshandler VM::syscalls[nsyscalls] = {
	{ NULL     , NULL          }, //  0
	{ "exit"   , &VM::_exit    }, //  1
	{ "fork"   , NULL          }, //  2
	{ "read"   , &VM::_read    }, //  3
	{ "write"  , &VM::_write   }, //  4
	{ "open"   , &VM::_open    }, //  5
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
	if (type < nsyscalls) {
		syshandler *sh = &syscalls[type];
		if (sh->name) {
			if (trace || !sh->f) {
				fprintf(stderr, "system call %s", sh->name);
			}
			if (sh->f) {
				(this->*sh->f)();
				AX = 0;
			} else {
				fprintf(stderr, ": not implemented\n");
				hasExited = true;
			}
			return;
		}
	}
	fprintf(stderr, "system call %d: unknown\n", type);
	hasExited = true;
}

void VM::_exit() {
	exitcode = read16(BX + 4);
	if (trace) fprintf(stderr, "(%d)\n", exitcode);
	hasExited = true;
}

void VM::_write() {
	int fd = read16(BX + 4);
	int buf = read16(BX + 10);
	int len = read16(BX + 6);
	if (trace) fprintf(stderr, "(%d, 0x%04x, %d)\n", fd, buf, len);
	int max = 0x10000 - buf;
	if (len > max) len = max;
	int result = write(fd, data + buf, len);
	write16(BX + 2, result == -1 ? -errno : result);
}

void VM::_read() {
	int fd = read16(BX + 4);
	int buf = read16(BX + 10);
	int len = read16(BX + 6);
	if (trace) fprintf(stderr, "(%d, 0x%04x, %d)\n", fd, buf, len);
	int max = 0x10000 - buf;
	if (len > max) len = max;
	int result = read(fd, data + buf, len);
	//fprintf(stderr, "result = %d\n", result);
	write16(BX + 2, result == -1 ? -errno : result);
}

void VM::_open() {
	int flag = read16(BX + 6);
	const char *path = (const char *)(data + read16(BX + (flag & 64 ? 10 : 8)));
	if (trace) fprintf(stderr, "(\"%s\", %d)\n", path, flag);
	std::string path2 = path; // TODO:convpath
#if WIN32
	flag |= O_BINARY;
#endif
	int result = open(path2.c_str(), flag);
	write16(BX + 2, result == -1 ? -errno : result);
	if (result != -1) {
		fd2name[result] = path2;
		handles.push_back(result);
	}
}

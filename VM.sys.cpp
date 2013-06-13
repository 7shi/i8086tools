#include "VM.h"
#include "disasm.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <map>
#include <algorithm>

std::map<int, std::string> fd2name;

#ifdef WIN32
std::list<std::string> unlinks;

static void showError(int err) {
	fprintf(stderr, "%s", getErrorMessage(err).c_str());
}
#endif

static int fileClose(VM *vm, int fd) {
	int ret = close(fd);
	std::map<int, std::string>::iterator it = fd2name.find(fd);
	if (it != fd2name.end())
	{
		std::string path = it->second;
		fd2name.erase(it);
#ifdef WIN32
		std::list<std::string>::iterator it2 =
			std::find(unlinks.begin(), unlinks.end(), path);
		if (it2 != unlinks.end())
		{
			if (trace)
				fprintf(stderr, "delayed unlink: %s\n", path.c_str());
			if (DeleteFileA(path.c_str()))
				unlinks.erase(it2);
			else if (trace)
				showError(GetLastError());
		}
#endif
	}
	return ret;
}

VM::syshandler VM::syscalls[nsyscalls] = {
	{ NULL         , NULL              }, //  0
	{ "exit"       , &VM::_exit        }, //  1
	{ "fork"       , NULL              }, //  2
	{ "read"       , &VM::_read        }, //  3
	{ "write"      , &VM::_write       }, //  4
	{ "open"       , &VM::_open        }, //  5
	{ "close"      , &VM::_close       }, //  6
	{ "wait"       , NULL              }, //  7
	{ "creat"      , &VM::_creat       }, //  8
	{ "link"       , NULL              }, //  9
	{ "unlink"     , &VM::_unlink      }, // 10
	{ "waitpid"    , NULL              }, // 11
	{ "chdir"      , NULL              }, // 12
	{ "time"       , NULL              }, // 13
	{ "mknod"      , NULL              }, // 14
	{ "chmod"      , NULL              }, // 15
	{ "chown"      , NULL              }, // 16
	{ "brk"        , &VM::_brk         }, // 17
	{ "stat"       , NULL              }, // 18
	{ "lseek"      , &VM::_lseek       }, // 19
	{ "getpid"     , &VM::_getpid      }, // 20
	{ "mount"      , NULL              }, // 21
	{ "umount"     , NULL              }, // 22
	{ "setuid"     , NULL              }, // 23
	{ "getuid"     , NULL              }, // 24
	{ "stime"      , NULL              }, // 25
	{ "ptrace"     , NULL              }, // 26
	{ "alarm"      , NULL              }, // 27
	{ "fstat"      , NULL              }, // 28
	{ "pause"      , NULL              }, // 29
	{ "utime"      , NULL              }, // 30
	{ NULL         , NULL              }, // 31
	{ NULL         , NULL              }, // 32
	{ "access"     , &VM::_access      }, // 33
	{ NULL         , NULL              }, // 34
	{ NULL         , NULL              }, // 35
	{ "sync"       , NULL              }, // 36
	{ "kill"       , NULL              }, // 37
	{ "rename"     , NULL              }, // 38
	{ "mkdir"      , NULL              }, // 39
	{ "rmdir"      , NULL              }, // 40
	{ "dup"        , NULL              }, // 41
	{ "pipe"       , NULL              }, // 42
	{ "times"      , NULL              }, // 43
	{ NULL         , NULL              }, // 44
	{ NULL         , NULL              }, // 45
	{ "setgid"     , NULL              }, // 46
	{ "getgid"     , NULL              }, // 47
	{ "signal"     , &VM::_signal      }, // 48
	{ NULL         , NULL              }, // 49
	{ NULL         , NULL              }, // 50
	{ NULL         , NULL              }, // 51
	{ NULL         , NULL              }, // 52
	{ NULL         , NULL              }, // 53
	{ "ioctl"      , NULL              }, // 54
	{ "fcntl"      , NULL              }, // 55
	{ NULL         , NULL              }, // 56
	{ NULL         , NULL              }, // 57
	{ NULL         , NULL              }, // 58
	{ "exec"       , NULL              }, // 59
	{ "umask"      , NULL              }, // 60
	{ "chroot"     , NULL              }, // 61
	{ "setsid"     , NULL              }, // 62
	{ "getpgrp"    , NULL              }, // 63
	{ "ksig"       , NULL              }, // 64
	{ "unpause"    , NULL              }, // 65
	{ NULL         , NULL              }, // 66
	{ "revive"     , NULL              }, // 67
	{ "task_reply" , NULL              }, // 68
	{ NULL         , NULL              }, // 69
	{ NULL         , NULL              }, // 70
	{ "sigaction"  , &VM::_sigaction   }, // 71
	{ "sigsuspend" , NULL              }, // 72
	{ "sigpending" , NULL              }, // 73
	{ "sigprocmask", NULL              }, // 74
	{ "sigreturn"  , NULL              }, // 75
	{ "reboot"     , NULL              }, // 76
	{ "svrctl"     , NULL              }, // 77
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

void VM::_exit() { // 1
	exitcode = read16(BX + 4);
	if (trace) fprintf(stderr, "(%d)\n", exitcode);
	hasExited = true;
	for (std::list<int>::iterator it = handles.begin(); it != handles.end(); ++it)
		fileClose(this, *it);
	handles.clear();
}

void VM::_read() { // 3
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

void VM::_write() { // 4
	int fd = read16(BX + 4);
	int buf = read16(BX + 10);
	int len = read16(BX + 6);
	if (trace) fprintf(stderr, "(%d, 0x%04x, %d)\n", fd, buf, len);
	int max = 0x10000 - buf;
	if (len > max) len = max;
	if (trace && fd < 3) { fflush(stdout); fflush(stderr); }
	int result = write(fd, data + buf, len);
	write16(BX + 2, result == -1 ? -errno : result);
}

void VM::_open() { // 5
	int flag = read16(BX + 6);
	const char *path = (const char *)(data + read16(BX + (flag & 64 ? 10 : 8)));
	if (trace) fprintf(stderr, "(\"%s\", %d)\n", path, flag);
	std::string path2 = convpath(path);
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

void VM::_close() { // 6
	int fd = read16(BX + 4);
	if (trace) fprintf(stderr, "(%d)\n", fd);
	int result = fileClose(this, fd);
	write16(BX + 2, result == -1 ? -errno : result);
	if (result == -1) handles.remove(result);
}

void VM::_creat() { // 8
	const char *path = (const char *)(data + read16(BX + 8));
	int mode = read16(BX + 6);
	if (trace) fprintf(stderr, "(\"%s\", 0%03o)\n", path, mode);
	std::string path2 = convpath(path);
	int result = creat(path2.c_str(), mode);
	write16(BX + 2, result == -1 ? -errno : result);
	if (result != -1) {
		fd2name[result] = path2;
		handles.push_back(result);
	}
}

void VM::_unlink() { // 10
	const char *path = (const char *)(data + read16(BX + 8));
	if (trace) fprintf(stderr, "(\"%s\")\n", path);
	std::string path2 = convpath(path);
#ifdef WIN32
	bool ok = DeleteFileA(path2.c_str());
	int err = 0;
	if (ok) {
		struct stat st;
		if (stat(path2.c_str(), &st) != -1) {
			if (trace) {
				fprintf(stderr, "register delayed: %s\n", path2.c_str());
			}
			unlinks.push_back(path2);
		}
	} else if (trace) {
		showError(err = GetLastError());
	}
	write16(BX + 2, -err);
#else
	int result = unlink(path2.c_str());
	write16(BX + 2, result == -1 ? -errno : result);
#endif
}

void VM::_brk() { // 17
	int nd = read16(BX + 10);
	if (trace) fprintf(stderr, "(0x%04x)\n", nd);
	if (nd < (int)dsize || nd >= SP) {
		write16(BX + 2, -ENOMEM);
	} else {
		write16(BX + 2, 0);
		write16(BX + 18, nd);
	}
}

void VM::_lseek() { // 19
	int fd = read16(BX + 4);
	off_t o = read32(BX + 10);
	int w = read16(BX + 6);
	if (trace) fprintf(stderr, "(%d, %ld, %d)\n", fd, o, w);
	int result = lseek(fd, o, w);
	write16(BX + 2, result == -1 ? -errno : result);
}

void VM::_getpid() { // 20
	if (trace) fprintf(stderr, "()\n");
	int result = getpid();
	write16(BX + 2, result == -1 ? -errno : result);
}

void VM::_access() { // 33
	const char *path = (const char *)(data + read16(BX + 8));
	int mode = read16(BX + 6);
	if (trace) fprintf(stderr, "(\"%s\", 0%03o)\n", path, mode);
	int result = access(path, mode);
	write16(BX + 2, result == -1 ? -errno : result);
}

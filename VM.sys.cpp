#include "VM.h"
#include "disasm.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#ifdef WIN32
#include <windows.h>
#define NO_FORK
#endif
#include <stack>
#include <map>
#include <algorithm>

#ifdef NO_FORK
static std::stack<std::pair<int, int> > exitcodes;
#endif

#ifdef WIN32
std::list<std::string> unlinks;

static void showError(int err) {
	fprintf(stderr, "%s", getErrorMessage(err).c_str());
}
#endif

int VM::close(int fd) {
	FileBase *f = file(fd);
	if (!f) return -1;

	files[fd] = NULL;
	if (--f->count) return 0;

	std::string path = f->path;
	delete f;

#ifdef WIN32
	std::list<std::string>::iterator it2 =
		std::find(unlinks.begin(), unlinks.end(), path);
	if (it2 != unlinks.end()) {
		if (trace)
			fprintf(stderr, "delayed unlink: %s\n", path.c_str());
		if (DeleteFileA(path.c_str()))
			unlinks.erase(it2);
		else if (trace)
			showError(GetLastError());
	}
#endif
	return 0;
}

VM::syshandler VM::syscalls[nsyscalls] = {
	{ NULL         , NULL              }, //  0
	{ "exit"       , &VM::_exit        }, //  1
	{ "fork"       , &VM::_fork        }, //  2
	{ "read"       , &VM::_read        }, //  3
	{ "write"      , &VM::_write       }, //  4
	{ "open"       , &VM::_open        }, //  5
	{ "close"      , &VM::_close       }, //  6
	{ "wait"       , &VM::_wait        }, //  7
	{ "creat"      , &VM::_creat       }, //  8
	{ "link"       , &VM::_link        }, //  9
	{ "unlink"     , &VM::_unlink      }, // 10
	{ "waitpid"    , NULL              }, // 11
	{ "chdir"      , NULL              }, // 12
	{ "time"       , &VM::_time        }, // 13
	{ "mknod"      , NULL              }, // 14
	{ "chmod"      , &VM::_chmod       }, // 15
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
	{ "fstat"      , &VM::_fstat       }, // 28
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
	{ "ioctl"      , &VM::_ioctl       }, // 54
	{ "fcntl"      , NULL              }, // 55
	{ NULL         , NULL              }, // 56
	{ NULL         , NULL              }, // 57
	{ NULL         , NULL              }, // 58
	{ "exec"       , &VM::_exec        }, // 59
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
				fprintf(stderr, "<%s", sh->name);
			}
			if (sh->f) {
				(this->*sh->f)();
				AX = 0;
			} else {
				fprintf(stderr, ": not implemented>\n");
				hasExited = true;
			}
			return;
		}
	}
	fprintf(stderr, "<%d: unknown syscall>\n", type);
	hasExited = true;
}

void VM::_exit() { // 1
	exitcode = (int16_t)read16(BX + 4);
	if (trace) fprintf(stderr, "(%d)>\n", exitcode);
#ifdef NO_FORK
	exitcodes.push(std::pair<int, int>(pid, exitcode));
#endif
	hasExited = true;
}

void VM::_fork() { // 2
	if (trace) fprintf(stderr, "()>\n");
#ifdef NO_FORK
	VM vm = *this;
	vm.write16(BX + 2, 0);
	vm.AX = 0;
	vm.run();
	write16(BX + 2, vm.pid);
#else
	int result = fork();
	write16(BX + 2, result == -1 ? -errno : result);
#endif
}

void VM::_read() { // 3
	int fd = read16(BX + 4);
	int buf = read16(BX + 10);
	int len = read16(BX + 6);
	if (trace) fprintf(stderr, "(%d, 0x%04x, %d)", fd, buf, len);
	int max = 0x10000 - buf;
	if (len > max) len = max;
	FileBase *f = file(fd);
	int result = f ? f->read(data + buf, len) : -1;
	write16(BX + 2, result == -1 ? -errno : result);
	if (trace) fprintf(stderr, " => %d>\n", result);
}

void VM::_write() { // 4
	int fd = read16(BX + 4);
	int buf = read16(BX + 10);
	int len = read16(BX + 6);
	if (trace) fprintf(stderr, "(%d, 0x%04x, %d)>\n", fd, buf, len);
	int max = 0x10000 - buf;
	if (len > max) len = max;
	FileBase *f = file(fd);
	int result = -1;
	if (f) {
		if (trace && f->fd < 3) { fflush(stdout); fflush(stderr); }
		result = f->write(data + buf, len);
	}
	write16(BX + 2, result == -1 ? -errno : result);
}

void VM::_open() { // 5
	int flag = read16(BX + 6);
	const char *path;
	mode_t mode = 0;
	if (flag & 64 /*O_CREAT*/) {
		path = (const char *)(data + read16(BX + 10));
		mode = read16(BX + 8);
		if (trace) fprintf(stderr, "(\"%s\", %d, 0%03o)", path, flag, mode);
	} else {
		path = (const char *)(data + read16(BX + 8));
		if (trace) fprintf(stderr, "(\"%s\", %d)", path, flag);
	}
	std::string path2 = convpath(path);
	int result = open(path2, flag, mode);
	write16(BX + 2, result == -1 ? -errno : result);
	if (trace) fprintf(stderr, " => %d>\n", result);
}

void VM::_close() { // 6
	int fd = read16(BX + 4);
	if (trace) fprintf(stderr, "(%d)", fd);
	int result = close(fd);
	write16(BX + 2, result == -1 ? -errno : result);
	if (trace) fprintf(stderr, " => %d>\n", result);
}

void VM::_wait() { // 7
	if (trace) fprintf(stderr, "()");
#ifdef NO_FORK
	if (!exitcodes.empty()) {
		std::pair<int, int> ec = exitcodes.top();
		exitcodes.pop();
		int status = ec.second;
		write16(BX + 2, ec.first);
		write16(BX + 4, status << 8);
		if (trace) fprintf(stderr, " => %d>\n", status);
	} else {
		write16(BX + 2, -EINVAL);
		if (trace) fprintf(stderr, " => EINVAL>\n");
	}
#else
	int status;
	int result = wait(&status);
	write16(BX + 2, result == -1 ? -errno : result);
	write16(BX + 4, status);
	if (trace) fprintf(stderr, " => 0x%04x>\n", status);
#endif
}

void VM::_creat() { // 8
	const char *path = (const char *)(data + read16(BX + 8));
	int mode = read16(BX + 6);
	if (trace) fprintf(stderr, "(\"%s\", 0%03o)", path, mode);
	std::string path2 = convpath(path);
#ifdef WIN32
	int result = open(path2, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, 0777);
#else
	int result = open(path2, O_CREAT | O_TRUNC | O_WRONLY, mode);
#endif
	write16(BX + 2, result == -1 ? -errno : result);
	if (trace) fprintf(stderr, " => %d>\n", result);
}

void VM::_link() { // 9
	const char *src = (const char *)(data + read16(BX + 10));
	const char *dst = (const char *)(data + read16(BX + 12));
	if (trace) fprintf(stderr, "(\"%s\", \"%s\")", src, dst);
#ifdef WIN32
	bool ok = CopyFileA(src, dst, TRUE);
	int err = ok ? 0 : GetLastError();
	if (trace) {
		fprintf(stderr, " => %d>\n", -err);
		if (!ok) showError(err);
	}
	write16(BX + 2, -err);
#else
	int result = link(convpath(src).c_str(), convpath(dst).c_str());
	write16(BX + 2, result == -1 ? -errno : result);
	if (trace) fprintf(stderr, " => %d>\n", result);
#endif
}

void VM::_unlink() { // 10
	const char *path = (const char *)(data + read16(BX + 8));
	if (trace) fprintf(stderr, "(\"%s\")", path);
	std::string path2 = convpath(path);
#ifdef WIN32
	bool ok = DeleteFileA(path2.c_str());
	int err = ok ? 0 : GetLastError();
	if (trace) fprintf(stderr, " => %d>\n", -err);
	if (!ok) {
		if (trace) showError(err);
		struct stat st;
		if (stat(path2.c_str(), &st) != -1) {
			if (trace) {
				fprintf(stderr, "<register delayed: %s>\n", path2.c_str());
			}
			unlinks.push_back(path2);
		}
	}
	write16(BX + 2, -err);
#else
	int result = unlink(path2.c_str());
	write16(BX + 2, result == -1 ? -errno : result);
	if (trace) fprintf(stderr, " => %d>\n", result);
#endif
}

void VM::_time() { // 13
	if (trace) fprintf(stderr, "()");
	time_t result = time(NULL);
	write16(BX + 2, result == -1 ? -errno : 0);
	write32(BX + 10, result);
	if (trace) fprintf(stderr, " => %ld>\n", result);
}

void VM::_chmod() { // 15
	const char *path = (const char *)(data + read16(BX + 8));
	int mode = read16(BX + 6);
	if (trace) fprintf(stderr, "(\"%s\", 0%03o)", path, mode);
	int result = chmod(convpath(path).c_str(), mode);
	write16(BX + 2, result == -1 ? -errno : result);
	if (trace) fprintf(stderr, " => %d>\n", result);
}

void VM::_brk() { // 17
	int nd = read16(BX + 10);
	if (trace) fprintf(stderr, "(0x%04x)", nd);
	if (nd < (int)dsize || nd >= ((SP - 0x1000) & ~0xfff)) {
		write16(BX + 2, -ENOMEM);
		if (trace) fprintf(stderr, " => ENOMEM>\n");
	} else {
		write16(BX + 2, 0);
		write16(BX + 18, nd);
		if (trace) fprintf(stderr, " => 0>\n");
	}
}

void VM::_lseek() { // 19
	int fd = read16(BX + 4);
	off_t o = read32(BX + 10);
	int w = read16(BX + 6);
	if (trace) fprintf(stderr, "(%d, %ld, %d)", fd, o, w);
	FileBase *f = file(fd);
	off_t result = -1;
	if (f) {
		result = f->lseek(o, w);
		if (result != -1) write32(BX + 10, result);
	}
	write16(BX + 2, result == -1 ? -errno : 0);
	if (trace) fprintf(stderr, " => %ld>\n", result);
}

void VM::_getpid() { // 20
	if (trace) fprintf(stderr, "()");
#ifdef NO_FORK
	int result = pid;
#else
	int result = getpid();
#endif
	write16(BX + 2, result == -1 ? -errno : result);
	if (trace) fprintf(stderr, " => %d>\n", result);
}

void VM::_fstat() { // 28
	int fd = read16(BX +  4);
	int p  = read16(BX + 10);
	if (trace) fprintf(stderr, "(%d, 0x%04x)", fd, p);
	struct stat st;
	FileBase *f = file(fd);
	int result = -1;
	if (f && f->fd > 2 && !(result = fstat(f->fd, &st))) {
		write16(p     , st.st_dev);
		write16(p +  2, st.st_ino);
		write16(p +  4, st.st_mode);
		write16(p +  6, st.st_nlink);
		write16(p +  8, st.st_uid);
		write16(p + 10, st.st_gid);
		write16(p + 12, st.st_rdev);
		write32(p + 14, st.st_size);
		write32(p + 18, st.st_atime);
		write32(p + 22, st.st_mtime);
		write32(p + 26, st.st_ctime);
	}
	write16(BX + 2, result == -1 ? -errno : 0);
	if (trace) fprintf(stderr, " => %d>\n", result);
}

void VM::_access() { // 33
	const char *path = (const char *)(data + read16(BX + 8));
	int mode = read16(BX + 6);
	if (trace) fprintf(stderr, "(\"%s\", 0%03o)", path, mode);
	std::string path2 = convpath(path);
	int result = access(path2.c_str(), mode);
	write16(BX + 2, result == -1 ? -errno : result);
	if (trace) fprintf(stderr, " => %d>\n", result);
}

void VM::_ioctl() { // 54
	int fd = read16(BX +  4);
	int rq = read16(BX +  8);
	int d  = read16(BX + 18);
	if (trace) fprintf(stderr, "(%d, 0x%04x, 0x%04x)>\n", fd, rq, d);
	write16(BX + 2, -EINVAL);
}

void VM::_exec() { // 59
	const char *path = (const char *)(data + read16(BX + 10));
	int fsize = read16(BX + 6);
	int frame = read16(BX + 12);
#if 0
	if (trace) fprintf(stderr, "(\"%s\", %d, 0x%04x)>\n", path, fsize, frame);
	FILE *f = fopen("core", "wb");
	fwrite(data, 1, 0x10000, f);
	fclose(f);
#endif
	int argc = read16(frame);
	if (trace) {
		fprintf(stderr, "(\"%s\"", path);
		for (int i = 2; i <= argc; i++) {
			fprintf(stderr, ", \"%s\"", data + frame + read16(frame + i * 2));
		}
		fprintf(stderr, ")>\n");
	}
	uint8_t *t = text, *d = data;
	text = new uint8_t[0x10000];
	memset(text, 0, 0x10000);
	data = NULL;
	if (!load(path)) {
		delete[] text;
		text = t;
		data = d;
		write16(BX + 2, -EINVAL);
		return;
	}
	start_sp = SP = 0x10000 - fsize;
	memcpy(data + start_sp, d + frame, fsize);
	if (d != t) delete[] d;
	int ad = start_sp + 2, p;
	for (int i = 0; i < 2; i++, ad += 2) {
		for(; (p = read16(ad)); ad += 2) {
			write16(ad, start_sp + p);
		}
	}
}

#pragma once
#include "utils.h"
#include "File.h"
#include <vector>
#include <list>
#ifdef WIN32
#define NO_FORK
#endif

extern int trace;
extern int exitcode;

class VMUnix {
protected:
    static VMUnix *current;
    uint8_t *text, *data;
    size_t tsize, dsize;
    uint16_t umask, brksize;
    bool hasExited;
    int pid;
    std::vector<FileBase *> files;

private:
    void init();

public:
    VMUnix();
    VMUnix(const VMUnix &vm);
    virtual ~VMUnix();
    virtual bool load(const std::string &fn) = 0;
    virtual void run(
            const std::vector<std::string> &args,
            const std::vector<std::string> &envs) = 0;
    virtual void run() = 0;
    virtual void disasm() = 0;

protected:
    virtual void setstat(uint16_t addr, struct stat *st) = 0;
    virtual bool syscall(int n) = 0;
    virtual int convsig(int sig) = 0;
    virtual void setsig(int sig, int h) = 0;
    virtual void swtch(bool reset = false) = 0;
    
    void swtch(VMUnix *to);

    inline uint8_t read8(uint16_t addr) {
        return data[addr];
    }

    inline uint16_t read16(uint16_t addr) {
        return ::read16(data + addr);
    }

    inline uint32_t read32(uint16_t addr) {
        return ::read32(data + addr);
    }

    inline void write8(uint16_t addr, uint8_t value) {
        data[addr] = value;
    }

    inline void write16(uint16_t addr, uint16_t value) {
        ::write16(data + addr, value);
    }

    inline void write32(uint16_t addr, uint32_t value) {
        ::write32(data + addr, value);
    }

    inline const char *str(uint16_t addr) {
        return (const char *) (data + addr);
    }

    int getfd();
    int open(const std::string &path, int flag, int mode);
    int close(int fd);
    FileBase *file(int fd);

    void sys_exit(int code);
    //int sys_fork();
    int sys_read(int fd, int buf, int len);
    int sys_write(int fd, int buf, int len);
    int sys_open(const char *path, int flag, mode_t mode = 0);
    int sys_close(int fd);
    int sys_wait(int *status);
    int sys_creat(const char *path, mode_t mode);
    int sys_link(const char *src, const char *dst);
    int sys_unlink(const char *path);
    int sys_time();
    int sys_chmod(const char *path, mode_t mode);
    int sys_brk(int nd, int sp);
    int sys_stat(const char *path, int p);
    off_t sys_lseek(int fd, off_t o, int w);
    int sys_getpid();
    int sys_getuid();
    int sys_fstat(int fd, int p);
    int sys_access(const char *path, mode_t mode);
    int sys_getgid();
    //void sys_signal();
    int sys_ioctl(int fd, int rq, int d);
    //int sys_exec(const char *path, int frame, int fsize);
    int sys_umask(mode_t mask);
    //void sys_sigaction();
};

#pragma once
#include "utils.h"
#include "OpCode.h"
#include "File.h"
#include <vector>
#include <list>
#ifdef WIN32
#define NO_FORK
#endif

#define AX r[0]
#define CX r[1]
#define DX r[2]
#define BX r[3]
#define SP r[4]
#define BP r[5]
#define SI r[6]
#define DI r[7]
#define AL *r8[0]
#define CL *r8[1]
#define DL *r8[2]
#define BL *r8[3]
#define AH *r8[4]
#define CH *r8[5]
#define DH *r8[6]
#define BH *r8[7]

extern bool ptable[256];
extern int trace;
extern int exitcode;
extern const char *header;

class VM {
protected:
    static VM *current;
    uint16_t ip, r[8];
    uint8_t *r8[8];
    uint8_t *text, *data;
    size_t tsize, dsize;
    bool OF, DF, SF, ZF, PF, CF;
    uint16_t start_sp, umask, brksize;
    bool hasExited;
    int pid;
    std::vector<FileBase *> files;
    std::vector<OpCode> cache;

private:
    void init();
public:
    VM();
    VM(const VM &vm);
    virtual ~VM();
    bool load(const std::string &fn);
    void run(
            const std::vector<std::string> &args,
            const std::vector<std::string> &envs);
    void run();
    void disasm();

protected:
    inline int setf8(int value, bool cf) {
        int8_t v = value;
        OF = value != v;
        SF = v < 0;
        ZF = v == 0;
        PF = ptable[uint8_t(value)];
        CF = cf;
        return value;
    }

    inline int setf16(int value, bool cf) {
        int16_t v = value;
        OF = value != v;
        SF = v < 0;
        ZF = v == 0;
        PF = ptable[uint8_t(value)];
        CF = cf;
        return value;
    }

    inline uint16_t read16(uint16_t addr) {
        return ::read16(data + addr);
    }

    inline uint32_t read32(uint16_t addr) {
        return ::read32(data + addr);
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

    void debug(uint16_t ip, const OpCode &op);
    int addr(const Operand &opr);
    uint8_t get8(const Operand &opr);
    uint16_t get16(const Operand &opr);
    void set8(const Operand &opr, uint8_t value);
    void set16(const Operand &opr, uint16_t value);
    void run1(uint8_t prefix = 0);
    int getfd();
    int open(const std::string &path, int flag, int mode);
    int close(int fd);
    FileBase *file(int fd);
    void setstat(uint16_t addr, struct stat *st);
    void swtch(VM *to);

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
    int sys_brk(int nd);
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

    virtual bool syscall(int n) = 0;

    static void sighandler(int sig);
    
    struct sigact {
        uint16_t handler;
        uint16_t mask;
        uint16_t flags;
    };
    static const int nsig = 12;
    sigact sigacts[nsig];

    virtual int convsig(int sig) = 0;
    virtual void setsig(int sig, int h) = 0;
};

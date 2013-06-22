#pragma once
#include "utils.h"
#include "OpCode.h"
#include "File.h"
#include <vector>
#include <list>

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
private:
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
    ~VM();
    bool load(const std::string &fn);
    void run(const std::vector<std::string> &args);
    void run();
    void disasm();

private:

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

    struct syshandler {
        const char *name;
        void (VM::*f)();
    };
    static const int nsyscalls = 78;
    static syshandler syscalls[nsyscalls];

    void minix_syscall();
    void _exit(); //  1
    void _fork(); //  2
    void _read(); //  3
    void _write(); //  4
    void _open(); //  5
    void _close(); //  6
    void _wait(); //  7
    void _creat(); //  8
    void _link(); //  9
    void _unlink(); // 10
    void _time(); // 13
    void _chmod(); // 15
    void _brk(); // 17
    void _stat(); // 18
    void _lseek(); // 19
    void _getpid(); // 20
    void _getuid(); // 24
    void _fstat(); // 28
    void _access(); // 33
    void _getgid(); // 47
    void _signal(); // 48
    void _ioctl(); // 54
    void _exec(); // 59
    void _umask(); // 60
    void _sigaction(); // 71

    static void sighandler(int sig);

    struct sigact {
        uint16_t sa_handler;
        uint16_t sa_mask;
        uint16_t sa_flags;
    };
    static const int nsig = 12;
    sigact sigacts[nsig];

    void swtch(VM *to);
};

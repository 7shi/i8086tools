#pragma once
#include "VM.h"
#include "Op8086.h"

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
extern const char *header;

class VM8086 : public VM {
protected:
    uint16_t ip, r[8];
    uint8_t *r8[8];
    bool OF, DF, SF, ZF, PF, CF;
    uint16_t start_sp;
    std::vector<Op8086> cache;

private:
    void init();

public:
    VM8086();
    VM8086(const VM8086 &vm);
    virtual ~VM8086();
    bool load(const std::string &fn);
    void run(
            const std::vector<std::string> &args,
            const std::vector<std::string> &envs);
    void run();
    void disasm();

    static Op8086 disasm1(uint8_t *mem, uint16_t addr);
    static Op8086 disasm1(uint8_t *mem, uint16_t addr, size_t last);
    static void disasm(uint8_t *mem, size_t size);

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

    uint8_t get8(const Opr8086 &opr);
    uint16_t get16(const Opr8086 &opr);
    void set8(const Opr8086 &opr, uint8_t value);
    void set16(const Opr8086 &opr, uint16_t value);

    void debug(uint16_t ip, const Op8086 &op);
    int addr(const Opr8086 &opr);
    void run1(uint8_t prefix = 0);
};

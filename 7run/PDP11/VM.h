#pragma once
#include "../VMUnix.h"
#include "OpCode.h"

namespace PDP11 {
    extern const char *header;

    class VM : public VMUnix {
    protected:
        uint16_t r[8];
        bool Z, N, C, V;
        uint16_t start_sp;
        std::vector<OpCode> cache;

    public:
        VM();
        VM(const VM &vm);
        virtual ~VM();
        virtual void disasm();

    protected:
        virtual bool load2(const std::string &fn, FILE *f);
        virtual void showHeader();
        virtual void run2();

        inline uint32_t getReg32(int reg) {
            return (r[reg] << 16) | r[(reg + 1) & 7];
        }

        inline void setReg32(int reg, uint32_t v) {
            r[reg] = v >> 16;
            r[(reg + 1) & 7] = v;
        }

        inline void setZNCV(bool z, bool n, bool c, bool v) {
            Z = z;
            N = n;
            C = c;
            V = v;
        }

        inline uint16_t getInc(const Operand &opr) {
            uint16_t ret = r[opr.reg];
            r[opr.reg] += opr.diff();
            return ret;
        }

        inline uint16_t getDec(const Operand &opr) {
            r[opr.reg] -= opr.diff();
            return r[opr.reg];
        }

        uint8_t get8(const Operand &opr, bool nomove = false);
        uint16_t get16(const Operand &opr, bool nomove = false);
        void set8(const Operand &opr, uint8_t value);
        void set16(const Operand &opr, uint16_t value);

        void debug(uint16_t pc, const OpCode &op);
        int addr(const Operand &opr, bool nomobe = false);
        void run1();
    };
}

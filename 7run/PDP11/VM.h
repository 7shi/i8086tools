#pragma once
#include "../VMUnix.h"
#include "OpCode.h"

namespace PDP11 {
    extern const char *header;

    bool check(uint8_t h[2]);

    class VM : public VMUnix {
    protected:
        uint16_t r[8];
        bool Z, N, C, V;
        uint16_t start_sp;
        std::vector<OpCode> cache;

    public:
        VM();
        virtual ~VM();
        virtual bool load(const std::string &fn);
        virtual void run(
                const std::vector<std::string> &args,
                const std::vector<std::string> &envs);
        virtual void run();
        virtual void disasm();

    protected:

        inline void setZNCV(bool z, bool n, bool c, bool v) {
            Z = z;
            N = n;
            C = c;
            V = v;
        }

        uint16_t getInc(const Operand &opr);
        uint16_t getDec(const Operand &opr);
        uint8_t get8(const Operand &opr, bool nomove = false);
        uint16_t get16(const Operand &opr, bool nomove = false);
        void set8(const Operand &opr, uint8_t value);
        void set16(const Operand &opr, uint16_t value);

        void debug(uint16_t ip, const OpCode &op);
        int addr(const Operand &opr, bool nomobe = false);
        void run1();
    };
}

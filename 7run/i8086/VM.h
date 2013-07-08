#pragma once
#include "../VMUnix.h"
#include "OpCode.h"

namespace i8086 {
    extern const char *header;

    class VM : public VMUnix {
    protected:
        uint16_t ip, r[8];
        uint8_t *r8[8];
        bool OF, DF, SF, ZF, PF, CF;
        uint16_t start_sp;
        std::vector<OpCode> cache;

    private:
        static bool ptable[256];
        void init();

    public:
        VM();
        VM(const VM &vm);
        virtual ~VM();
        virtual void disasm();

    protected:
        virtual bool loadInternal(const std::string &fn, FILE *f);
        virtual void showHeader();
        virtual void setArgs(
                const std::vector<std::string> &args,
                const std::vector<std::string> &envs);
        virtual void runInternal();

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

        uint8_t get8(const Operand &opr);
        uint16_t get16(const Operand &opr);
        void set8(const Operand &opr, uint8_t value);
        void set16(const Operand &opr, uint16_t value);

        void debug(uint16_t ip, const OpCode &op);
        int addr(const Operand &opr);
        void run1(uint8_t prefix = 0);
    };
}

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
    };
}

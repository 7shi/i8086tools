#pragma once
#include "../VMBase.h"

namespace PDP11 {
    extern const char *header;

    class VMPDP11 : public VMBase {
    public:
        VMPDP11();
        virtual ~VMPDP11();
        virtual bool load(const std::string &fn);
        virtual void run(
                const std::vector<std::string> &args,
                const std::vector<std::string> &envs);
        virtual void run();
        virtual void disasm();

        virtual void setstat(uint16_t addr, struct stat *st);
        virtual bool syscall(int n);
        virtual int convsig(int sig);
        virtual void setsig(int sig, int h);
    };
}

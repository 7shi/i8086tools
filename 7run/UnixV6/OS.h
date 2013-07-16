#pragma once
#include "OSBase.h"
#include "../PDP11/VM.h"

namespace UnixV6 {
    bool check(uint8_t h[2]);

    class OS : public OSBase {
    private:
        PDP11::VM cpu;

    public:
        OS();
        OS(const OS &os);
        virtual ~OS();

        virtual bool syscall(int n);

    protected:
        virtual void setArgs(
                const std::vector<std::string> &args,
                const std::vector<std::string> &envs);
        virtual bool load2(const std::string &fn, FILE *f, size_t size);

    public:
        virtual int v6_fork(); //  2
        virtual int v6_wait(); // 7
        virtual int v6_exec(const char *path, int argp); // 11
        virtual int v6_brk(int nd); // 17

    protected:
        virtual void sighandler2(int sig);
    };
}

#pragma once
#include "../UnixBase.h"
#include "../PDP11/VM.h"

namespace UnixV6 {
    bool check(uint8_t h[2]);

    class OS : public UnixBase {
    private:
        PDP11::VM cpu;

    public:
        OS();
        OS(const OS &vm);
        virtual ~OS();

        virtual void disasm();
        virtual bool syscall(int n);

    protected:
        virtual void setArgs(
                const std::vector<std::string> &args,
                const std::vector<std::string> &envs);
        virtual bool load2(const std::string &fn, FILE *f, size_t size);
        virtual void setstat(uint16_t addr, struct stat *st);
        virtual int convsig(int sig);
        virtual void setsig(int sig, int h);
        virtual void swtch(bool reset = false);

    private:
        bool syscall(int n, uint8_t *args);
        int v6_fork(); //  2
        int v6_exec(const char *path, int args); // 11
        int v6_seek(int fd, off_t o, int w); // 19
        int v6_signal(int sig, int h); // 48

        static void sighandler(int sig);
        void sighandler2(int sig);
        void resetsig();

        static const int nsig = 20;
        uint16_t sighandlers[nsig];
    };
}

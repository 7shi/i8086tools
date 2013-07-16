#pragma once
#include "../UnixBase.h"

namespace UnixV6 {
    bool check(uint8_t h[2]);

    class OSBase : public UnixBase {
    public:
        OSBase();
        OSBase(const OSBase &os);
        virtual ~OSBase();

        virtual void disasm();

    protected:
        virtual void setstat(uint16_t addr, struct stat *st);
        virtual int convsig(int sig);
        virtual void setsig(int sig, int h);
        virtual void swtch(bool reset = false);

        int syscall(int *result, int n, int arg0, uint8_t *args);

    public:
        virtual int v6_fork() = 0; //  2
        virtual int v6_wait() = 0; // 7
        virtual int v6_exec(const char *path, int argp) = 0; // 11
        virtual int v6_brk(int nd) = 0; // 17
        int v6_seek(int fd, off_t o, int w); // 19
        int v6_signal(int sig, int h); // 48

    protected:
        static void sighandler(int sig);
        virtual void sighandler2(int sig) = 0;
        void resetsig();

        static const int nsig = 20;
        uint16_t sighandlers[nsig];
    };
}

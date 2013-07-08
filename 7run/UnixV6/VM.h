#pragma once
#include "../PDP11/VM.h"

namespace UnixV6 {
    bool check(uint8_t h[2]);

    class VM : public PDP11::VM {
    public:
        VM();
        VM(const VM &vm);
        virtual ~VM();

    protected:
        virtual bool load2(const std::string &fn, FILE *f);
        virtual void setstat(uint16_t addr, struct stat *st);
        virtual bool syscall(int n);
        virtual int convsig(int sig);
        virtual void setsig(int sig, int h);
        virtual void swtch(bool reset = false);

    private:
        bool syscall(int n, uint8_t *args);
        int v6_fork(); //  2
        int v6_exec(); // 11
        int v6_signal(); // 48

        static void sighandler(int sig);
        void sighandler2(int sig);
        void resetsig();
    };
}

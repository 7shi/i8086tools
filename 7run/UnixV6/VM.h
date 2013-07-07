#pragma once
#include "../PDP11/VM.h"

namespace UnixV6 {

    class VM : public PDP11::VM {
    public:
        VM();
        VM(const VM &vm);
        virtual ~VM();

    protected:
        virtual bool syscall(int n);
        virtual void setsig(int sig, int h);
        virtual void setstat(uint16_t addr, struct stat *st);
        virtual void swtch(bool reset = false);

    private:
        bool syscall(int n, uint8_t *args);
        int v6_fork(); //  2
        int v6_exec(); // 11
        int v6_signal(); // 48

        static void sighandler(int sig);
        void sighandler2(int sig);
        virtual int convsig(int sig);
        void resetsig();
    };
}

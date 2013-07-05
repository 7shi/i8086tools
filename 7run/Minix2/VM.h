#pragma once
#include "../i8086/VM.h"

namespace Minix2 {

    class VM : public i8086::VM {
    public:
        VM();
        VM(const VM &vm);
        virtual ~VM();

    protected:
        virtual bool syscall(int n);
        virtual void setsig(int sig, int h);
        virtual void setstat(uint16_t addr, struct stat *st);

    private:
        int minix_fork(); //  2
        int minix_signal(); // 48
        int minix_exec(); // 59
        int minix_sigaction(); // 71

        static void sighandler(int sig);
        void sighandler2(int sig);
        virtual int convsig(int sig);
        void resetsig();
    };
}
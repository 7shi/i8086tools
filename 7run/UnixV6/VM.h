#pragma once
#include "../PDP11/VM.h"

namespace UnixV6 {
    class VM : public PDP11::VM {
    public:
        VM();
        virtual ~VM();
        virtual void setstat(uint16_t addr, struct stat *st);
        virtual bool syscall(int n);
        virtual int convsig(int sig);
        virtual void setsig(int sig, int h);
    };
}

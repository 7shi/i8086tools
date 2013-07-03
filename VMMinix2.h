#pragma once
#include "VM.h"

class VMMinix2: public VM {

public:
    VMMinix2();
    VMMinix2(const VMMinix2 &vm);
    virtual ~VMMinix2();

protected:
    virtual bool syscall(int n);
    virtual void setsig(int sig, int h);

private:
    int minix_fork(); //  2
    int minix_signal(); // 48
    int minix_exec(); // 59
    int minix_sigaction(); // 71

    virtual int convsig(int sig);
    void resetsig();
};

#pragma once
#include "i8086/VM8086.h"

class VMMinix2 : public i8086::VM8086 {
public:
    VMMinix2();
    VMMinix2(const VMMinix2 &vm);
    virtual ~VMMinix2();

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

#include "VM.h"
#include <stdio.h>

using namespace UnixV6;

VM::VM() {
}

VM::~VM() {
}

void VM::setstat(uint16_t addr, struct stat *st) {
    fprintf(stderr, "[%s] not implemented\n", __func__);
}

bool VM::syscall(int n) {
    fprintf(stderr, "[%s] not implemented\n", __func__);
    return false;
}

int VM::convsig(int sig) {
    fprintf(stderr, "[%s] not implemented\n", __func__);
    return 0;
}

void VM::setsig(int sig, int h) {
    fprintf(stderr, "[%s] not implemented\n", __func__);
}

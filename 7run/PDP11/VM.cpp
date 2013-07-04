#include "VM.h"
#include <stdio.h>

bool PDP11::check(uint8_t h[2]) {
    int magic = ::read16(h);
    return magic == 0407 || magic == 0410 || magic == 0411;
}

using namespace PDP11;

VM::VM() {
}

VM::~VM() {
}

bool VM::load(const std::string &fn) {
    fprintf(stderr, "not implemented\n");
    return false;
}

void VM::run(
        const std::vector<std::string> &args,
        const std::vector<std::string> &envs) {
    fprintf(stderr, "not implemented\n");
}

void VM::run() {
    fprintf(stderr, "not implemented\n");
}

void VM::disasm() {
    fprintf(stderr, "not implemented\n");
}

void VM::setstat(uint16_t addr, struct stat *st) {
    fprintf(stderr, "not implemented\n");
}

bool VM::syscall(int n) {
    fprintf(stderr, "not implemented\n");
    return false;
}

int VM::convsig(int sig) {
    fprintf(stderr, "not implemented\n");
    return 0;
}

void VM::setsig(int sig, int h) {
    fprintf(stderr, "not implemented\n");
}

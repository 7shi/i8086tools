#include "VMPDP11.h"
#include <stdio.h>

using namespace PDP11;

VMPDP11::VMPDP11() {
}

VMPDP11::~VMPDP11() {
}

bool VMPDP11::load(const std::string &fn) {
    fprintf(stderr, "not implemented\n");
    return false;
}

void VMPDP11::run(
        const std::vector<std::string> &args,
        const std::vector<std::string> &envs) {
    fprintf(stderr, "not implemented\n");
}

void VMPDP11::run() {
    fprintf(stderr, "not implemented\n");
}

void VMPDP11::disasm() {
    fprintf(stderr, "not implemented\n");
}

void VMPDP11::setstat(uint16_t addr, struct stat *st) {
    fprintf(stderr, "not implemented\n");
}

bool VMPDP11::syscall(int n) {
    fprintf(stderr, "not implemented\n");
    return false;
}

int VMPDP11::convsig(int sig) {
    fprintf(stderr, "not implemented\n");
    return 0;
}

void VMPDP11::setsig(int sig, int h) {
    fprintf(stderr, "not implemented\n");
}

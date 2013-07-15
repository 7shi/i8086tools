#include "VMBase.h"
#include "VMUnix.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int trace;
int exitcode;

VMBase::VMBase() : data(NULL), tsize(0), brksize(0), hasExited(false) {
    text = new uint8_t[0x10000];
    memset(text, 0, 0x10000);
}

VMBase::VMBase(const VMBase &vm) : hasExited(false) {
    text = new uint8_t[0x10000];
    memcpy(text, vm.text, 0x10000);
    if (vm.data == vm.text) {
        data = text;
    } else {
        data = new uint8_t[0x10000];
        memcpy(data, vm.data, 0x10000);
    }
    tsize = vm.tsize;
    dsize = vm.dsize;
    brksize = vm.brksize;
}

VMBase::~VMBase() {
    if (data != text) delete[] data;
    delete[] text;
}

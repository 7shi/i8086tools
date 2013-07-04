#include "VM.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

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
    std::string fn2 = convpath(fn);
    const char *file = fn2.c_str();
    struct stat st;
    if (stat(file, &st)) {
        fprintf(stderr, "can not stat: %s\n", file);
        return false;
    }
    tsize = st.st_size;
    FILE *f = fopen(file, "rb");
    if (!f) {
        fprintf(stderr, "can not open: %s\n", file);
        return false;
    }
    if (tsize >= 0x10) {
        uint8_t h[0x10];
        if (fread(h, sizeof (h), 1, f) && check(h)) {
            tsize = ::read16(h + 2);
            dsize = ::read16(h + 4);
            //PC = ::read16(h + 10);
            if (h[0] == 9) { // 0411
                //cache.clear();
                //cache.resize(0x10000);
                data = new uint8_t[0x10000];
                memset(data, 0, 0x10000);
                fread(text, 1, tsize, f);
                fread(data, 1, dsize, f);
            } else {
                //cache.clear();
                data = text;
                if (h[0] == 8) { // 0410
                    fread(text, 1, tsize, f);
                    uint16_t doff = (tsize + 0x1fff) & ~0x1fff;
                    fread(text + doff, 1, dsize, f);
                    dsize += doff;
                } else {
                    dsize += tsize;
                    fread(text, 1, dsize, f);
                }
            }
            dsize += ::read16(h + 6); // bss
            brksize = dsize;
        } else {
            fseek(f, 0, SEEK_SET);
        }
    }
    if (!data) {
        if (tsize > 0xffff) {
            fprintf(stderr, "too long raw binary: %s\n", file);
            fclose(f);
            return false;
        }
        //cache.clear();
        data = text;
        fread(text, 1, tsize, f);
        brksize = dsize = tsize;
    }
    fclose(f);
    return true;
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

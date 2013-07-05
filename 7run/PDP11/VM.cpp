#include "VM.h"
#include "disasm.h"
#include "regs.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

using namespace PDP11;

bool PDP11::check(uint8_t h[2]) {
    int magic = ::read16(h);
    return magic == 0407 || magic == 0410 || magic == 0411;
}

VM::VM() : start_sp(0) {
    memset(r, 0, sizeof (r));
    Z = N = C = V = false;
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
    int slen = 0;
    for (int i = 0; i < (int) args.size(); i++) {
        slen += args[i].size() + 1;
    }
    SP -= (slen + 1) & ~1;
    uint16_t ad1 = SP;
    SP -= (1 + args.size()) * 2;
    uint16_t ad2 = start_sp = SP;
    write16(SP, args.size()); // argc
    for (int i = 0; i < (int) args.size(); i++) {
        write16(ad2 += 2, ad1);
        strcpy((char *) data + ad1, args[i].c_str());
        ad1 += args[i].size() + 1;
    }
    run();
}

void VM::run() {
    fprintf(stderr, "[%s] not implemented\n", __func__);
}

void VM::disasm() {
    ::disasm(text, tsize);
}

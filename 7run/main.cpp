#include "Minix2/OS.h"
#include "UnixV6/OS.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    bool dis = false, pdp11 = false;
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-r") {
            i++;
            if (i < argc) setroot(argv[i]);
        } else if (arg == "-m") {
            trace = 3;
        } else if (arg == "-v") {
            trace = 2;
        } else if (arg == "-s" && trace == 0) {
            trace = 1;
        } else if (arg == "-d") {
            dis = true;
        } else if (arg == "-p") {
            pdp11 = true;
        } else {
            for (; i < argc; i++) {
                args.push_back(argv[i]);
            }
        }
    }
    if (args.empty()) {
        printf("usage: %s [-p] [-d|-v/-s] cmd [args ...]\n", argv[0]);
        printf("    -p: PDP-11 mode\n");
        printf("    -d: disassemble mode (not run)\n");
        printf("    -m: verbose mode with memory dump\n");
        printf("    -v: verbose mode (output syscall and disassemble)\n");
        printf("    -s: syscall mode (output syscall)\n");
        return 1;
    }

    uint8_t buf[2];
    FILE *f = fopen(convpath(args[0]).c_str(), "rb");
    if (!f) {
        fprintf(stderr, "can not open: %s\n", args[0].c_str());
        return 1;
    }
    if (fread(buf, 1, 2, f) != 2) {
        fprintf(stderr, "can not read: %s\n", args[0].c_str());
        fclose(f);
        return 1;
    }

    UnixBase *vm;
    if (pdp11 || UnixV6::check(buf)) {
        vm = new UnixV6::OS();
    } else {
        vm = new Minix2::OS();
    }

    if (!vm->load(args[0])) {
        exitcode = 1;
    } else if (dis) {
        vm->disasm();
    } else {
        exitcode = 0;
        std::vector<std::string> envs;
        envs.push_back("PATH=/bin:/usr/bin");
        vm->run(args, envs);
    }
    delete vm;
    return exitcode;
}

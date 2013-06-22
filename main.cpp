#include "VM.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    bool dis = false;
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
        } else {
            for (; i < argc; i++) {
                args.push_back(argv[i]);
            }
        }
    }
    if (args.empty()) {
        printf("usage: %s [-d|-v/-s] cmd [args ...]\n", argv[0]);
        printf("    -d: disassemble mode (not run)\n");
        printf("    -m: verbose mode with memory dump\n");
        printf("    -v: verbose mode (output syscall and disassemble)\n");
        printf("    -s: syscall mode (output syscall)\n");
        return 1;
    }
    VM vm;
    if (!vm.load(args[0])) return 1;
    if (dis) {
        vm.disasm();
    } else {
        if (trace == 2) fprintf(stderr, header);
        exitcode = 0;
        vm.run(args);
    }
    return exitcode;
}

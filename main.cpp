#include "VM.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
	bool dis = false;
	std::vector<std::string> args;
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
		if (arg == "-v") {
			trace = 2;
		} else if (arg == "-s" && trace == 0) {
			trace = 1;
		} else {
			for (; i < argc; i++) {
				args.push_back(argv[i]);
			}
		}
	}
	if (args.empty())
	{
		printf("usage: %s [-d|-v/-s] cmd [args ...]\n", argv[0]);
		printf("    -d: disassemble mode (not run)\n");
		printf("    -v: verbose mode (output syscall and disassemble)\n");
		printf("    -s: syscall mode (output syscall)\n");
		return 1;
	}
	VM vm;
	if (!vm.load(args[0])) return 1;
	if (dis) {
		vm.disasm();
	} else {
		vm.run(args);
	}
	return vm.exitcode;
}

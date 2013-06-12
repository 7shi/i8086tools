#include "VM.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
	bool dis = false;
	const char *file = NULL;
	std::vector<std::string> args;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-d")) {
			dis = true;
		} else if (!strcmp(argv[i], "-v")) {
			verbose = true;
		} else {
			file = argv[i];
			for (; i < argc; i++) {
				args.push_back(argv[i]);
			}
		}
	}
	if (!file) {
		fprintf(stderr, "usage: %s [-d|-v] binary\n", argv[0]);
		return 1;
	}
	VM vm;
	if (!vm.read(file)) return 1;
	if (dis) {
		vm.disasm();
	} else {
		vm.run(args);
	}
	return vm.exit_status;
}

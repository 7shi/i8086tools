#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <vector>
#include <string>

struct OpCode {
	size_t len;
	std::string mne;

	OpCode(int len, const std::string &mne)
		: len(len), mne(mne) {}
};

int undefined;

OpCode disasm(uint8_t *p) {
	undefined++;
	return OpCode(1, "(undefined)");
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s binary\n", argv[0]);
		return 1;
	}
	struct stat st;
	if (stat(argv[1], &st)) {
		fprintf(stderr, "can not stat: %s\n", argv[1]);
		return 1;
	}
	FILE *f = fopen(argv[1], "rb");
	if (!f) {
		fprintf(stderr, "can not open: %s\n", argv[1]);
		return 1;
	}
	std::vector<uint8_t> aout(st.st_size);
	fread(&aout[0], 1, st.st_size, f);
	fclose(f);

	off_t index = 0;
	while (index < aout.size()) {
		OpCode op = disasm(&aout[index]);
		std::string hex;
		char buf[3];
		for (int i = 0; i < op.len; i++) {
			snprintf(buf, sizeof(buf), "%02x", aout[index + i]);
			hex += buf;
		}
		printf("%04x: %-10s %s\n", index, hex.c_str(), op.mne.c_str());
		index += op.len;
	}
	printf("undefined: %d\n", undefined);

	return 0;
}

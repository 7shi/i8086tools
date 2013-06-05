#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <vector>
#include <string>

struct OpCode {
	size_t len;
	std::string mne;

	OpCode(): len(0) {}
	OpCode(int len, const std::string &mne)
		: len(len), mne(mne) {}
	inline bool empty() { return len == 0; }
};

int undefined;

OpCode disasm0(uint8_t *p) {
	switch (p[0]) {
	case 0x27: return OpCode(1,"baa");
	case 0x2F: return OpCode(1,"das");
	case 0x37: return OpCode(1,"aaa");
	case 0x3F: return OpCode(1,"aas");
	case 0x98: return OpCode(1,"cbw");
	case 0x99: return OpCode(1,"cwd");
	case 0x9B: return OpCode(1,"wait");
	case 0x9C: return OpCode(1,"pushf");
	case 0x9D: return OpCode(1,"popf");
	case 0x9E: return OpCode(1,"sahf");
	case 0x9F: return OpCode(1,"lahf");
	case 0xC3: return OpCode(1,"ret");
	case 0xCB: return OpCode(1,"retf");
	case 0xCE: return OpCode(1,"into");
	case 0xCF: return OpCode(1,"iret");
	case 0xD7: return OpCode(1,"xlat");
	case 0xF0: return OpCode(1,"lock");
	case 0xF4: return OpCode(1,"hlt");
	case 0xF5: return OpCode(1,"cmc");
	case 0xF8: return OpCode(1,"clc");
	case 0xF9: return OpCode(1,"stc");
	case 0xFA: return OpCode(1,"cli");
	case 0xFB: return OpCode(1,"sti");
	case 0xFC: return OpCode(1,"cld");
	case 0xFD: return OpCode(1,"std");
	}
	return OpCode();
}

OpCode disasm(uint8_t *p) {
	OpCode ret = disasm0(p);
	if (!ret.empty()) return ret;
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

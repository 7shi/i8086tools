#include "disasm.h"
#include <stdio.h>
#include <sys/stat.h>
#include <vector>

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
	size_t size = st.st_size;
	FILE *f = fopen(argv[1], "rb");
	if (!f) {
		fprintf(stderr, "can not open: %s\n", argv[1]);
		return 1;
	}
	if (size >= 0x20) {
		uint8_t h[0x20];
		int hdrlen = 0;
		if (fread(h, sizeof(h), 1, f)) {
			hdrlen = h[4];
			size = read32(h + 8);
		}
		fseek(f, hdrlen, SEEK_SET);
	}
	std::vector<uint8_t> aout(size + 16);
	fread(&aout[0], 1, size, f);
	fclose(f);
	disasm(&aout[0], size);
	return 0;
}

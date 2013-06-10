#include "disasm.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

uint16_t ip;
uint8_t text[65536], mem[65536], *data;

int main(int argc, char *argv[]) {
	bool dis = false;
	const char *file = NULL;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-d")) {
			dis = true;
		} else if (i + 1 == argc) {
			file = argv[i];
		} else {
			break;
		}
	}
	if (!file) {
		fprintf(stderr, "usage: %s [-d] binary\n", argv[0]);
		return 1;
	}
	struct stat st;
	if (stat(file, &st)) {
		fprintf(stderr, "can not stat: %s\n", file);
		return 1;
	}
	size_t size = st.st_size;
	FILE *f = fopen(file, "rb");
	if (!f) {
		fprintf(stderr, "can not open: %s\n", file);
		return 1;
	}
	if (size >= 0x20) {
		uint8_t h[0x20];
		if (fread(h, sizeof(h), 1, f) && h[0] == 1 && h[1] == 3
			&& !fseek(f, h[4], SEEK_SET)) {
			if (h[3] != 4) {
				fprintf(stderr, "unknown cpu id: %d\n", h[3]);
				fclose(f);
				return 1;
			}
			size = read32(h + 8);
			int dsize = read32(h + 12);
			ip = read32(h + 20);
			if (h[2] & 0x20) {
				data = mem;
				fread(text, 1,  size, f);
				fread(data, 1, dsize, f);
			} else {
				data = text;
				fread(text, 1, size + dsize, f);
			}
		} else {
			fseek(f, 0, SEEK_SET);
		}
	}
	if (!data) {
		if (size > sizeof(text)) {
			fprintf(stderr, "too long raw binary: %s\n", file);
			fclose(f);
			return 1;
		}
		data = text;
		fread(text, 1, size, f);
	}
	fclose(f);
	if (dis) {
		disasm(text, size);
	} else {
		printf("entry: %04x\n", ip);
	}
	return 0;
}

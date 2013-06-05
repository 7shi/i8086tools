#include <stdio.h>
#include <stdint.h>
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
	FILE *f = fopen(argv[1], "rb");
	if (!f) {
		fprintf(stderr, "can not open: %s\n", argv[1]);
		return 1;
	}
	std::vector<uint8_t> aout(st.st_size);
	fread(&aout[0], 1, st.st_size, f);
	fclose(f);
	return 0;
}

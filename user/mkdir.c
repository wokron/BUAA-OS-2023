#include <lib.h>

void usage(void) {
	printf("usage: mkdir [dir...]\n");
	exit();
}

int main(int argc, char *argv[]) {
	int r;
	int rt = 0;
	struct Stat stat_buf;
	if (argc < 2) {
		usage();
	}

	for (int i = 1; i < argc; i++) {
		if (stat(argv[i], &stat_buf) >= 0) {
			printf("%s already exists\n", argv[i]);
			rt = -1;
			continue;
		}
		if ((r = mkdir(argv[i])) < 0) {
			printf("cannot create dir %s\n", argv[i]);
			rt = r;
		}
	}

	return rt;
}

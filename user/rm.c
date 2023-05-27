#include <lib.h>

void usage(void) {
	printf("usage: rm file...\n");
	exit();
}

int main(int argc, char *argv[]) {
	int rt = 0;
	int r;

	if (argc < 2) {
		usage();
	}

	for (int i = 1; i < argc; i++) {
		if ((r = remove(argv[i])) < 0) {
			printf("fail to remove %s\n", argv[i]);
			rt = r;
		}
	}

	return rt;
}

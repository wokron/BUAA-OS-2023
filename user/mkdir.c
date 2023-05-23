#include <lib.h>

void usage(void) {
	printf("usage: mkdir [dir]\n");
	exit();
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		usage();
	}
	return mkdir(argv[1]);
}

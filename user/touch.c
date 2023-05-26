#include <lib.h>

void usage(void) {
	printf("usage: touch file...\n");
	exit();
}

int main(int argc, char *argv[]) {
	int fd;
	if (argc < 2) {
		usage();
	}
	
	for (int i = 1; i < argc; i++) {
		if ((fd = open(argv[i], O_CREAT)) >= 0) {
			close(fd);
		}
	}
	
	return 0;
}

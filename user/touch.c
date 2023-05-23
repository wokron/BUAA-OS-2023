#include <lib.h>

void usage(void) {
	printf("touch [file]\n");
	exit();
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		usage();
	}
	
	int fd = open(argv[1], O_CREAT);
	close(fd);
	
	return 0;
}

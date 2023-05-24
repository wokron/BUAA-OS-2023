#include <lib.h>

int main(int argc, char *argv[]) {
	int fd;
	if ((fd = open("/.history", O_RDONLY | O_CREAT)) < 0) {
		user_panic("open .history, fd = %d", fd);
	}
	
	char buf[1024];
	int r;
	while ((r = read(fd, buf, 1024)) > 0) {
		buf[r] = '\0';
		printf("%s", buf);
	}

	close(fd);
}

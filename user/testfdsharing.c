#include <lib.h>

char buf[512], buf2[512];

int memcmp(char *a, char *b, int n) {
	int i;

	for (i = 0; i < n; i++) {
		if (a[i] != b[i]) {
			return 1;
		}
	}
	return 0;
}

int main(int argc, char **argv) {
	int fd, r, n, n2;

	if ((fd = open("motd", O_RDONLY)) < 0) {
		user_panic("open motd: %d", fd);
	}
	seek(fd, 0);
	if ((n = readn(fd, buf, sizeof buf)) <= 0) {
		user_panic("readn: %d", n);
	}

	if ((r = fork()) < 0) {
		user_panic("fork: %d", r);
	}
	if (r == 0) {
		seek(fd, 0);
		debugf("going to read in child (might panic if your sharing is buggy)\n");
		if ((n2 = readn(fd, buf2, sizeof buf2)) != n) {
			user_panic("read in parent got %d, read in child got %d", n, n2);
		}
		if (memcmp(buf, buf2, n) != 0) {
			user_panic("read in parent got different bytes from read in child");
		}
		debugf("read in child succeeded\n");
		seek(fd, 0);
		close(fd);
		exit();
	}
	wait(r);
	// seek(fd, 0);
	if ((n2 = readn(fd, buf2, sizeof buf2)) != n) {
		user_panic("read in parent got %d, then got %d", n, n2);
	}
	debugf("buf : %s\n", buf);
	debugf("read in parent succeeded\n");
	return 0;
}

#include <lib.h>

int main() {
	int dirfd, filefd;
	int r;

	if ((r = open("/test_dir", O_RDONLY)) < 0) {
		user_panic("open dir failed: %d", r);
	}
	dirfd = r;

	if ((r = openat(dirfd, "test_file", O_RDWR)) < 0) {
		user_panic("openat file filed: %d", r);
	}
	filefd = r;

	char buf[1024];
	char *str = "successfully open file!";
	int len = 23;
	int l = read(r, buf, len);
	if (l != len) {
		user_panic("read file failed");
	}
	buf[len] = '\0';

	if (strcmp(str, buf) != 0) {
		debugf("test openat failed\n");
		return -1;
	}

	debugf("successfully openat file!\n");

	close(filefd);
	close(dirfd);
	return 0;
}

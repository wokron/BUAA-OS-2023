#include <lib.h>

static char *msg = "This is the NEW message of the day!\n";
static char *diff_msg = "This is a different message of the day!\n";

int main() {
	int r;
	int fdnum;
	char buf[512];
	int n;

	if ((r = open("/newmotd", O_RDWR)) < 0) {
		user_panic("open /newmotd: %d", r);
	}
	fdnum = r;
	debugf("open is good\n");

	if ((n = read(fdnum, buf, 511)) < 0) {
		user_panic("read /newmotd: %d", r);
	}
	if (strcmp(buf, diff_msg) != 0) {
		user_panic("read returned wrong data");
	}
	debugf("read is good\n");

	if ((r = ftruncate(fdnum, 0)) < 0) {
		user_panic("ftruncate: %d", r);
	}
	seek(fdnum, 0);

	if ((r = write(fdnum, msg, strlen(msg) + 1)) < 0) {
		user_panic("write /newmotd: %d", r);
	}

	if ((r = close(fdnum)) < 0) {
		user_panic("close /newmotd: %d", r);
	}

	// read again
	if ((r = open("/newmotd", O_RDONLY)) < 0) {
		user_panic("open /newmotd: %d", r);
	}
	fdnum = r;
	debugf("open again: OK\n");

	if ((n = read(fdnum, buf, 511)) < 0) {
		user_panic("read /newmotd: %d", r);
	}
	if (strcmp(buf, msg) != 0) {
		user_panic("read returned wrong data");
	}
	debugf("read again: OK\n");

	if ((r = close(fdnum)) < 0) {
		user_panic("close /newmotd: %d", r);
	}
	debugf("file rewrite is good\n");
	if ((r = remove("/newmotd")) < 0) {
		user_panic("remove /newmotd: %d", r);
	}
	if ((r = open("/newmotd", O_RDONLY)) >= 0) {
		user_panic("open after remove /newmotd: %d", r);
	}
	debugf("file remove: OK\n");
	return 0;
}

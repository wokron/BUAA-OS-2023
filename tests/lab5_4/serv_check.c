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
		user_panic("read /newmotd: %d", n);
	}
	if (strcmp(buf, diff_msg) != 0) {
		user_panic("read returned wrong data");
	}
	debugf("read is good\n");

	if ((r = ftruncate(fdnum, 0)) < 0) {
		user_panic("ftruncate: %d", r);
	}
	seek(fdnum, 0);

	int next_n = strlen(msg) + 1;
	if ((r = write(fdnum, msg, next_n) < 0)) {
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

	if ((r = open("/newmotd", O_RDONLY)) < 0) {
		user_panic("open /newmotd: %d", r);
	}
	fdnum = r;
	memset(buf, 0xfe, sizeof buf);
	if ((n = read(fdnum, buf, sizeof buf)) != next_n) {
		user_panic("read /newmotd: %d (%d expected)", n, next_n);
	}
	if (buf[next_n - 1] != 0) {
		user_panic("read /newmotd: zero byte not written");
	}
	int k = 0;
	for (k = next_n; k < sizeof buf; k++) {
		if ((u_char)buf[k] != 0xfe) {
			user_panic("read /newmotd: buffer overflow: %x", (u_char)buf[k]);
		}
	}
	if ((r = close(fdnum)) < 0) {
		user_panic("close /newmotd: %d", r);
	}

	debugf("1 OK\n");

	// test fd:
	for (k = 0; k < 31; k++) {
		if ((r = open("/newmotd", O_RDWR)) < 0) {
			user_panic("not enough fd");
		}
		if (r != k) {
			user_panic("fdnum mismatch");
		}
	}

	for (k = 30; k >= 0; k--) {
		if ((r = close(k)) < 0) {
			user_panic("cannot close fd");
		}
	}

	if ((r = open("/newmotd", O_RDWR)) < 0 || r != 0) {
		user_panic("close without free fd");
	}

	if ((n = read(fdnum, buf, 511)) < 0) {
		user_panic("read /newmotd: %d", r);
	}

	if ((r = close(fdnum)) < 0) {
		user_panic("canot close fd");
	}

	if ((n = read(fdnum, buf, 511)) > 0) {
		user_panic("read on a closed file: %d", r);
	}

	if ((r = open("/newmotd", O_WRONLY)) < 0 || r != 0) {
		user_panic("close without free fd");
	}

	if ((r = close(fdnum)) < 0) {
		user_panic("cannot close fd");
	}

	debugf("2 OK\n");

	// test limit of autority:
	fdnum = r;
	if ((n = read(fdnum, buf, 511)) >= 0) {
		user_panic("read on a file withou permission");
	}

	if ((r = open("/newmotd", O_WRONLY)) < 0 || r != 0) {
		user_panic("close without free fd");
	}
	fdnum = r;

	if ((n = read(fdnum, buf, 511)) >= 0) {
		user_panic("read on a file withou permission: %d", r);
	}

	if ((r = close(fdnum)) < 0) {
		user_panic("cannot close fd");
	}

	if ((r = open("/newmotd", O_RDONLY)) < 0 || r != 0) {
		user_panic("close without free fd");
	}
	fdnum = r;

	if ((r = write(fdnum, msg, strlen(msg) + 1)) >= 0) {
		user_panic("write on a file without permission d: %d", r);
	}

	debugf("3 OK\n");

	// test remove
	if ((r = close(fdnum)) < 0) {
		user_panic("cannot close fd");
	}

	if ((r = remove("/newmotd")) < 0) {
		user_panic("remove /newmotd: %d", r);
	}
	if ((r = open("/newmotd", O_RDONLY)) >= 0) {
		user_panic("open after remove /newmotd: %d", r);
	}
	debugf("file remove: OK\n");

	debugf("4 OK\n");

	syscall_read_dev(&r, 0x10000010, 4);
	return 0;
}

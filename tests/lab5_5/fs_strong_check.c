#include <lib.h>

static char *file1 = "/newmotd";
static char *file2 = "/motd";
static char *file3 = "/bin/test";
static char *file4 = "/etc/profile";

void test_read(char *pth, char *str) {
	char buf[1024];
	int r = open(pth, O_RDWR);
	if (r < 0) {
		user_panic("failed to open %s, return value: %d", pth, r);
	}
	int len = strlen(str);
	fprintf(r, str, len);
	close(r);
	r = open(pth, O_RDWR);
	if (r < 0) {
		user_panic("failed to open %s, return value: %d", pth, r);
	}
	int l = read(r, buf, len);
	if (l != len) {
		user_panic("wrong length, expect: %d, we got: %d", len, l);
	}
	buf[len] = '\0';
	if (strcmp(str, buf) != 0) {
		debugf("test read failed, expect: %s, we got: %s\n", str, buf);
		return;
	}
	close(r);
	debugf("passed test_read with strlen=%d, file=%s\n", strlen(str), pth);
}

void test_fd_alloc(char *pth) {
	int k = 0, r = 0;
	for (k = 0; k < 31; k++) {
		if ((r = open(pth, O_RDWR)) < 0) {
			user_panic("not enough fd");
		}
		if (r != k) {
			user_panic("fdnum mismatch, expect: %d, we got: %d", k, r);
		}
	}

	for (k = 30; k >= 0; k--) {
		if ((r = close(k)) < 0) {
			user_panic("cannot close fd");
		}
	}

	debugf("pass fd alloc test with file=%s\n", pth);
}

void test_close(char *pth) {
	int r, fdnum, n;
	char buf[512];

	if ((r = open(pth, O_RDWR)) < 0 || r != 0) {
		user_panic("close without free fd");
	}
	fdnum = r;
	if ((n = read(fdnum, buf, 511)) < 0) {
		user_panic("read /newmotd: %d", r);
	}

	if ((r = close(fdnum)) < 0) {
		user_panic("canot close fd");
	}

	if ((n = read(fdnum, buf, 511)) > 0) {
		user_panic("read on a closed file: %d", r);
	}

	if ((r = open(pth, O_WRONLY)) < 0 || r != 0) {
		user_panic("close without free fd");
	}

	if ((r = close(fdnum)) < 0) {
		user_panic("canot close fd");
	}
	debugf("pass close test with file=%s\n", pth);
}

void test_mode(char *pth) {
	int r, fdnum, n;
	char buf[512];
	buf[0] = 'a';

	if ((r = open(pth, O_WRONLY)) < 0 || r != 0) {
		user_panic("close without free fd");
	}
	fdnum = r;

	if ((n = read(fdnum, buf, 511)) >= 0) {
		user_panic("read on a file withou permission: %d", r);
	}

	if ((r = close(fdnum)) < 0) {
		user_panic("canot close fd");
	}

	if ((r = open(pth, O_RDONLY)) < 0 || r != 0) {
		user_panic("close without free fd");
	}
	fdnum = r;

	if ((r = write(fdnum, buf, strlen(buf) + 1)) >= 0) {
		user_panic("write on a file without permission d: %d", r);
	}

	close(fdnum);

	debugf("pass permission test with file=%s\n", pth);
}

void test_remove(char *pth) {
	int r, fdnum;

	if ((r = open(pth, O_WRONLY)) < 0 || r != 0) {
		user_panic("close without free fd");
	}
	fdnum = r;

	if ((r = close(fdnum)) < 0) {
		user_panic("canot close fd");
	}

	if ((r = remove(pth)) < 0) {
		user_panic("remove %d", r);
	}
	if ((r = open(pth, O_RDONLY)) >= 0) {
		user_panic("open after remove: %d", r);
	}
	debugf("pass remove test with file=%s\n", pth);
}

int main() {
	char test_str1[1 + 1];
	char test_str2[128 + 1];
	char test_str3[512 + 1];
	int i = 0;
	for (i = 0; i < 1; i++) {
		test_str1[i] = 'a';
	}
	test_str1[1] = 0;

	for (i = 0; i < 128; i++) {
		test_str2[i] = 'b';
	}

	test_str2[128] = 0;

	for (i = 0; i < 512; i++) {
		test_str3[i] = 'c';
	}
	test_str3[512] = 0;

	char *files[4] = {file1, file2, file3, file4};
	for (i = 0; i < 4; i++) {
		// debugf("file:%s\n", files[i]);
		test_read(files[i], test_str1);
		test_read(files[i], test_str2);
		test_read(files[i], test_str3);
	}

	for (i = 0; i < 4; i++) {
		test_fd_alloc(files[i]);
	}

	for (i = 0; i < 4; i++) {
		test_close(files[i]);
	}

	for (i = 0; i < 4; i++) {
		test_mode(files[i]);
	}

	for (i = 0; i < 4; i++) {
		test_remove(files[i]);
	}
	int r;
	syscall_read_dev(&r, 0x10000010, 4);
	return 0;
}

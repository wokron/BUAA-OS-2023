#include <lib.h>

int main() {
	debugf("devtst begin\n");
	int i = 0;
	int r;
	char buf[32] __attribute__((aligned(4))) = {0};
	char c __attribute__((aligned(4))) = 0;
	u_int cons = 0x10000000;
	while (1) {
		if ((r = syscall_read_dev(&c, cons, 1)) != 0) {
			debugf("syscall_read_dev is bad\n");
		}
		if (c == '\r') {
			break;
		}
		if (c != 0) {
			buf[i++] = c;
		}
	}
	if (i == 14) {
		debugf("syscall_read_dev is good\n");
	}
	buf[i++] = '\n';
	for (int j = 0; j < i; j++) {
		int ch = buf[j];
		if ((r = syscall_write_dev(&ch, cons, 4)) != 0) {
			debugf("syscall_write_dev is bad\n");
		}
	}
	debugf("end of devtst\n");

	if (syscall_read_dev(&c, 0x0fffffff, 1) != -3) {
		user_panic("failed dev address test");
	}
	if (syscall_read_dev(&c, 0x10000020, 1) != -3) {
		user_panic("failed dev address test");
	}
	if (syscall_read_dev(&c, 0x1000001f, 8) != -3) {
		user_panic("failed dev address test");
	}

	if (syscall_read_dev(&c, 0x12ffffff, 1) != -3) {
		user_panic("failed dev address test");
	}
	if (syscall_read_dev(&c, 0x13004200, 1) != -3) {
		user_panic("failed dev address test");
	}
	if (syscall_read_dev(&c, 0x130041ff, 8) != -3) {
		user_panic("failed dev address test");
	}

	if (syscall_read_dev(&c, 0x14ffffff, 1) != -3) {
		user_panic("failed dev address test");
	}
	if (syscall_read_dev(&c, 0x15000200, 1) != -3) {
		user_panic("failed dev address test");
	}
	if (syscall_read_dev(&c, 0x150001ff, 8) != -3) {
		user_panic("failed dev address test");
	}

	if (syscall_write_dev(&c, 0x0fffffff, 1) != -3) {
		user_panic("failed dev address test");
	}
	if (syscall_write_dev(&c, 0x10000020, 1) != -3) {
		user_panic("failed dev address test");
	}
	if (syscall_write_dev(&c, 0x1000001f, 8) != -3) {
		user_panic("failed dev address test");
	}

	if (syscall_write_dev(&c, 0x12ffffff, 1) != -3) {
		user_panic("failed dev address test");
	}
	if (syscall_write_dev(&c, 0x13004200, 1) != -3) {
		user_panic("failed dev address test");
	}
	if (syscall_write_dev(&c, 0x130041ff, 8) != -3) {
		user_panic("failed dev address test");
	}

	if (syscall_write_dev(&c, 0x14ffffff, 1) != -3) {
		user_panic("failed dev address test");
	}
	if (syscall_write_dev(&c, 0x15000200, 1) != -3) {
		user_panic("failed dev address test");
	}
	if (syscall_write_dev(&c, 0x150001ff, 8) != -3) {
		user_panic("failed dev address test");
	}

	debugf("dev address is ok\n");

	syscall_read_dev(&c, 0x10000010, 4);
	return 0;
}

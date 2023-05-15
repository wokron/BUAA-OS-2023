#include "../fs/serv.h"
#include <lib.h>

void ssd_write_number(u_int logic_no, int num) {
	char src[512];
	memset(src, 0, sizeof(src));
	*(int *)src = num;
	ssd_write(logic_no, src);
}

int ssd_read_number(u_int logic_no) {
	char dst[512];
	ssd_read(logic_no, dst);
	return *(int *)dst;
}

int main() {
	char dst[512];

	ssd_init();
	for (int i = 0; i < 16; i++) {
		ssd_write_number(i, i * i);
	}
	for (int i = 0; i < 16; i++) {
		if (ssd_read_number(i) != i * i) {
			user_panic("READ_ERROR");
		}
	}

	// 覆写检查1
	int prev = 3;
	for (int i = 1; i <= 16; i++) {
		ssd_write_number(3, i * 10 + 3);
		if (ssd_read_number(3) != i * 10 + 3) {
			user_panic("OVERWRITE_ERROR");
		}

		ide_read(0, 15 + i, dst, 1);
		if (*(int *)dst != i * 10 + 3) {
			user_panic("OVERWRITE_ERROR");
		}

		ide_read(0, prev, dst, 1);
		if (*(int *)dst != 0) {
			user_panic("ERASE_ERROR");
		}

		prev = 15 + i;
	}

	// 覆写检查2
	ssd_write_number(10, 110);
	ssd_write_number(11, 111);
	ssd_write_number(12, 112);

	ide_read(0, 3, dst, 1);
	if (*(int *)dst != 110) {
		user_panic("OVERWRITE_ERROR");
	}

	ide_read(0, 10, dst, 1);
	if (*(int *)dst != 111) {
		user_panic("OVERWRITE_ERROR");
	}

	ide_read(0, 11, dst, 1);
	if (*(int *)dst != 112) {
		user_panic("OVERWRITE_ERROR");
	}

	ide_read(0, 12, dst, 1);
	if (*(int *)dst != 0) {
		user_panic("ERASE_ERROR");
	}

	// 擦除检查
	ssd_erase(6);
	ssd_erase(7);

	ide_read(0, 6, dst, 1);
	if (*(int *)dst != 0) {
		user_panic("ERASE_ERROR");
	}

	ide_read(0, 7, dst, 1);
	if (*(int *)dst != 0) {
		user_panic("ERASE_ERROR");
	}

	// 检查ssd_read返回值
	if (ssd_read(15, dst) != 0) {
		user_panic("RETVAL_ERROR");
	}
	if (ssd_read(16, dst) != -1) {
		user_panic("RETVAL_ERROR");
	}
	debugf("SSD_CHECK PASSED!\n");
	return 0;
}

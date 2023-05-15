/*
 * operations on IDE disk.
 */

#include "serv.h"
#include <drivers/dev_disk.h>
#include <lib.h>
#include <mmu.h>

// Overview:
//  read data from IDE disk. First issue a read request through
//  disk register and then copy data from disk buffer
//  (512 bytes, a sector) to destination array.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  dst: destination for data read from IDE disk.
//  nsecs: the number of sectors to read.
//
// Post-Condition:
//  Panic if any error occurs. (you may want to use 'panic_on')
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_OPERATION_READ',
//  'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS', 'DEV_DISK_BUFFER'
void ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		/* Exercise 5.3: Your code here. (1/2) */
		syscall_write_dev(&temp, DEV_DISK_ADDRESS | DEV_DISK_ID, 4);
		temp = begin + off;
		syscall_write_dev(&temp, DEV_DISK_ADDRESS | DEV_DISK_OFFSET, 4);
		temp = DEV_DISK_OPERATION_READ;
		syscall_write_dev(&temp, DEV_DISK_ADDRESS | DEV_DISK_START_OPERATION, 4);
		syscall_read_dev(&temp, DEV_DISK_ADDRESS | DEV_DISK_STATUS, 4);
		if (temp == 0) {
			panic_on("fail to read from disk");
		}
		syscall_read_dev(dst + off, DEV_DISK_ADDRESS | DEV_DISK_BUFFER, BY2SECT);
	}
}

// Overview:
//  write data to IDE disk.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  src: the source data to write into IDE disk.
//  nsecs: the number of sectors to write.
//
// Post-Condition:
//  Panic if any error occurs.
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_BUFFER',
//  'DEV_DISK_OPERATION_WRITE', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS'
void ide_write(u_int diskno, u_int secno, void *src, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		/* Exercise 5.3: Your code here. (2/2) */
		syscall_write_dev(&temp, DEV_DISK_ADDRESS | DEV_DISK_ID, 4);
		temp = begin + off;
		syscall_write_dev(&temp, DEV_DISK_ADDRESS | DEV_DISK_OFFSET, 4);
		syscall_write_dev(src + off, DEV_DISK_ADDRESS | DEV_DISK_BUFFER, BY2SECT);
		temp = DEV_DISK_OPERATION_WRITE;
		syscall_write_dev(&temp, DEV_DISK_ADDRESS | DEV_DISK_START_OPERATION, 4);
		syscall_read_dev(&temp, DEV_DISK_ADDRESS| DEV_DISK_STATUS, 4);
		if (temp == 0) {
			panic_on("fail to write disk");
		}
	}
}

// this is bitmap
u_int writable;

struct SSDPhysics {
	u_int write_times;
};

struct SSDMap {
	u_int p_no;
	int is_empty;
};

struct SSDPhysics ssd_p[32];
struct SSDMap ssd_map[32];

void ssd_init() {
	for (int i = 0; i < 32; i++) {
		ssd_map[i].p_no = -1;
		ssd_map[i].is_empty = 1;
	}

	for (int i = 0; i < 32; i++) {
		ssd_p[i].write_times = 0;
	}

	writable = 0xffffffff;
}
int ssd_read(u_int logic_no, void *dst) {
	struct SSDMap map = ssd_map[logic_no];

	if (map.is_empty) {
		return -1;
	}

	ide_read(0, map.p_no, dst, 1);
	return 0;
}

char zeros[1024] = {0};
void ssd_erase_physical(u_int physical_no) {
	ide_write(0, physical_no, zeros, 1);
	ssd_p[physical_no].write_times++;
	writable |= (1 << physical_no);
}

u_int ssd_alloc() {
	int a_idx = -1;
	for (int i = 0; i < 32; i++) {
		if ((writable & (1<<i)) &&
		(a_idx == -1 || ssd_p[i].write_times < ssd_p[a_idx].write_times)) {
			a_idx = i;
		}
	}

	struct SSDPhysics* a = ssd_p + a_idx;

	if (a->write_times < 5) {
		return a_idx;
	}

	int b_idx = -1;
	for (int i = 0; i < 32; i++) {
		if (!(writable & (1<<i)) &&
		(b_idx == -1 || ssd_p[i].write_times < ssd_p[b_idx].write_times)) {
			b_idx = i;
		}
	}

	struct SSDPhysics* b = ssd_p + b_idx;
	char buf[1024];
	// from b to a
	ide_read(0, b_idx, buf, 1); // read from b
	ide_write(0, a_idx, buf, 1); // write to a
//	ssd_read(b_idx, buf);
//	ssd_write(a_idx, buf);
//	a->write_times++;
	writable &= ~(1 << a_idx);

	for (int i = 0; i < 32; i++) {
		if (ssd_map[i].p_no == b_idx) {
			ssd_map[i].p_no = a_idx;
			break;
		}
	}

	ssd_erase_physical(b_idx);

	
	return b_idx;
}

void ssd_write(u_int logic_no, void *src) {
	struct SSDMap* map = ssd_map + logic_no;

	if (!map->is_empty) {
		ssd_erase_physical(map->p_no);
		map->is_empty = 1;
		map->p_no = -1;
	}

	u_int new_p_no = ssd_alloc();
	
	map->p_no = new_p_no;
	
	map->is_empty = 0;
	
	ide_write(0, map->p_no, src, 1);
	
	writable &= ~(1 << map->p_no);
}
void ssd_erase(u_int logic_no) {
	struct SSDMap* map = ssd_map + logic_no;

	if (map->is_empty) {
		return;
	}

	ssd_erase_physical(map->p_no);
	map->is_empty = 1;
	map->p_no = -1;
}

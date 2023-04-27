#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BY2PG 4096 // bytes to a page
#include "../user/include/fs.h"

/* Static assert, for compile-time assertion checking */
#define static_assert(c) (void)(char(*)[(c) ? 1 : -1])0

#define nelem(x) (sizeof(x) / sizeof((x)[0]))
typedef struct Super Super;
typedef struct File File;

#define NBLOCK 1024 // The number of blocks in the disk.
uint32_t nbitblock; // the number of bitmap blocks.
uint32_t nextbno;   // next availiable block.

struct Super super; // super block.

enum {
	BLOCK_FREE = 0,
	BLOCK_BOOT = 1,
	BLOCK_BMAP = 2,
	BLOCK_SUPER = 3,
	BLOCK_DATA = 4,
	BLOCK_FILE = 5,
	BLOCK_INDEX = 6,
};

struct Block {
	uint8_t data[BY2BLK];
	uint32_t type;
} disk[NBLOCK];

// reverse: mutually transform between little endian and big endian.
void reverse(uint32_t *p) {
	uint8_t *x = (uint8_t *)p;
	uint32_t y = *(uint32_t *)x;
	x[3] = y & 0xFF;
	x[2] = (y >> 8) & 0xFF;
	x[1] = (y >> 16) & 0xFF;
	x[0] = (y >> 24) & 0xFF;
}

// reverse_block: reverse proper filed in a block.
void reverse_block(struct Block *b) {
	int i, j;
	struct Super *s;
	struct File *f, *ff;
	uint32_t *u;

	switch (b->type) {
	case BLOCK_FREE:
	case BLOCK_BOOT:
		break; // do nothing.
	case BLOCK_SUPER:
		s = (struct Super *)b->data;
		reverse(&s->s_magic);
		reverse(&s->s_nblocks);

		ff = &s->s_root;
		reverse(&ff->f_size);
		reverse(&ff->f_type);
		for (i = 0; i < NDIRECT; ++i) {
			reverse(&ff->f_direct[i]);
		}
		reverse(&ff->f_indirect);
		break;
	case BLOCK_FILE:
		f = (struct File *)b->data;
		for (i = 0; i < FILE2BLK; ++i) {
			ff = f + i;
			if (ff->f_name[0] == 0) {
				break;
			} else {
				reverse(&ff->f_size);
				reverse(&ff->f_type);
				for (j = 0; j < NDIRECT; ++j) {
					reverse(&ff->f_direct[j]);
				}
				reverse(&ff->f_indirect);
			}
		}
		break;
	case BLOCK_INDEX:
	case BLOCK_BMAP:
		u = (uint32_t *)b->data;
		for (i = 0; i < BY2BLK / 4; ++i) {
			reverse(u + i);
		}
		break;
	}
}

// Initial the disk. Do some work with bitmap and super block.
void init_disk() {
	int i, diff;

	// Step 1: Mark boot sector block.
	disk[0].type = BLOCK_BOOT;

	// Step 2: Initialize boundary.
	nbitblock = (NBLOCK + BIT2BLK - 1) / BIT2BLK;
	nextbno = 2 + nbitblock;

	// Step 2: Initialize bitmap blocks.
	for (i = 0; i < nbitblock; ++i) {
		disk[2 + i].type = BLOCK_BMAP;
	}
	for (i = 0; i < nbitblock; ++i) {
		memset(disk[2 + i].data, 0xff, BY2BLK);
	}
	if (NBLOCK != nbitblock * BIT2BLK) {
		diff = NBLOCK % BIT2BLK / 8;
		memset(disk[2 + (nbitblock - 1)].data + diff, 0x00, BY2BLK - diff);
	}

	// Step 3: Initialize super block.
	disk[1].type = BLOCK_SUPER;
	super.s_magic = FS_MAGIC;
	super.s_nblocks = NBLOCK;
	super.s_root.f_type = FTYPE_DIR;
	strcpy(super.s_root.f_name, "/");
}

// Get next block id, and set `type` to the block's type.
int next_block(int type) {
	disk[nextbno].type = type;
	return nextbno++;
}

// Flush disk block usage to bitmap.
void flush_bitmap() {
	int i;
	// update bitmap, mark all bit where corresponding block is used.
	for (i = 0; i < nextbno; ++i) {
		((uint32_t *)disk[2 + i / BIT2BLK].data)[(i % BIT2BLK) / 32] &= ~(1 << (i % 32));
	}
}

// Finish all work, dump block array into physical file.
void finish_fs(char *name) {
	int fd, i;

	// Prepare super block.
	memcpy(disk[1].data, &super, sizeof(super));

	// Dump data in `disk` to target image file.
	fd = open(name, O_RDWR | O_CREAT, 0666);
	for (i = 0; i < 1024; ++i) {
#ifdef CONFIG_REVERSE_ENDIAN
		reverse_block(disk + i);
#endif
		ssize_t n = write(fd, disk[i].data, BY2BLK);
		assert(n == BY2BLK);
	}

	// Finish.
	close(fd);
}

// Save block link.
void save_block_link(struct File *f, int nblk, int bno) {
	assert(nblk < NINDIRECT); // if not, file is too large !

	if (nblk < NDIRECT) {
		f->f_direct[nblk] = bno;
	} else {
		if (f->f_indirect == 0) {
			// create new indirect block.
			f->f_indirect = next_block(BLOCK_INDEX);
		}
		((uint32_t *)(disk[f->f_indirect].data))[nblk] = bno;
	}
}

// Make new block contians link to files in a directory.
int make_link_block(struct File *dirf, int nblk) {
	int bno = next_block(BLOCK_FILE);
	save_block_link(dirf, nblk, bno);
	dirf->f_size += BY2BLK;
	return bno;
}

// Overview:
//  Allocate an unused 'struct File' under the specified directory.
//
//  Note that when we delete a file, we do not re-arrenge all
//  other 'File's, so we should reuse existing unused 'File's here.
//
// Post-Condition:
//  Return a pointer to an unused 'struct File'.
//  We assume that this function will never fail.
//
// Hint:
//  Use 'make_link_block' to allocate a new block for the directory if there are no existing unused
//  'File's.
struct File *create_file(struct File *dirf) {
	int nblk = dirf->f_size / BY2BLK;

	// Step 1: Iterate through all existing blocks in the directory.
	for (int i = 0; i < nblk; ++i) {
		int bno; // the block number
		// If the block number is in the range of direct pointers (NDIRECT), get the 'bno'
		// directly from 'f_direct'. Otherwise, access the indirect block on 'disk' and get
		// the 'bno' at the index.
		/* Exercise 5.5: Your code here. (1/3) */

		// Get the directory block using the block number.
		struct File *blk = (struct File *)(disk[bno].data);

		// Iterate through all 'File's in the directory block.
		for (struct File *f = blk; f < blk + FILE2BLK; ++f) {
			// If the first byte of the file name is null, the 'File' is unused.
			// Return a pointer to the unused 'File'.
			/* Exercise 5.5: Your code here. (2/3) */

		}
	}

	// Step 2: If no unused file is found, allocate a new block using 'make_link_block' function
	// and return a pointer to the new block on 'disk'.
	/* Exercise 5.5: Your code here. (3/3) */

	return NULL;
}

// Write file to disk under specified dir.
void write_file(struct File *dirf, const char *path) {
	int iblk = 0, r = 0, n = sizeof(disk[0].data);
	struct File *target = create_file(dirf);

	/* in case `create_file` is't filled */
	if (target == NULL) {
		return;
	}

	int fd = open(path, O_RDONLY);

	// Get file name with no path prefix.
	const char *fname = strrchr(path, '/');
	if (fname) {
		fname++;
	} else {
		fname = path;
	}
	strcpy(target->f_name, fname);

	target->f_size = lseek(fd, 0, SEEK_END);
	target->f_type = FTYPE_REG;

	// Start reading file.
	lseek(fd, 0, SEEK_SET);
	while ((r = read(fd, disk[nextbno].data, n)) > 0) {
		save_block_link(target, iblk++, next_block(BLOCK_DATA));
	}
	close(fd); // Close file descriptor.
}

// Overview:
//  Write directory to disk under specified dir.
//  Notice that we may use POSIX library functions to operate on
//  directory to get file infomation.
//
// Post-Condition:
//  We ASSUME that this funcion will never fail
void write_directory(struct File *dirf, char *path) {
	DIR *dir = opendir(path);
	if (dir == NULL) {
		perror("opendir");
		return;
	}
	struct File *pdir = create_file(dirf);
	strncpy(pdir->f_name, basename(path), MAXNAMELEN - 1);
	if (pdir->f_name[MAXNAMELEN - 1] != 0) {
		fprintf(stderr, "file name is too long: %s\n", path);
		// File already created, no way back from here.
		exit(1);
	}
	pdir->f_type = FTYPE_DIR;
	for (struct dirent *e; (e = readdir(dir)) != NULL;) {
		if (strcmp(e->d_name, ".") != 0 && strcmp(e->d_name, "..") != 0) {
			char *buf = malloc(strlen(path) + strlen(e->d_name) + 2);
			sprintf(buf, "%s/%s", path, e->d_name);
			if (e->d_type == DT_DIR) {
				write_directory(pdir, buf);
			} else {
				write_file(pdir, buf);
			}
			free(buf);
		}
	}
	closedir(dir);
}

int main(int argc, char **argv) {
	static_assert(sizeof(struct File) == BY2FILE);
	init_disk();

	if (argc < 3) {
		fprintf(stderr, "Usage: fsformat <img-file> [files or directories]...\n");
		exit(1);
	}

	for (int i = 2; i < argc; i++) {
		char *name = argv[i];
		struct stat stat_buf;
		int r = stat(name, &stat_buf);
		assert(r == 0);
		if (S_ISDIR(stat_buf.st_mode)) {
			printf("writing directory '%s' recursively into disk\n", name);
			write_directory(&super.s_root, name);
		} else if (S_ISREG(stat_buf.st_mode)) {
			printf("writing regular file '%s' into disk\n", name);
			write_file(&super.s_root, name);
		} else {
			fprintf(stderr, "'%s' has illegal file mode %o\n", name, stat_buf.st_mode);
			exit(2);
		}
	}

	flush_bitmap();
	finish_fs(argv[1]);

	return 0;
}

#include <lib.h>
#include <mmu.h>

static int cons_read(struct Fd *, void *, u_int, u_int);
static int cons_write(struct Fd *, const void *, u_int, u_int);
static int cons_close(struct Fd *);
static int cons_stat(struct Fd *, struct Stat *);

struct Dev devcons = {
    .dev_id = 'c',
    .dev_name = "cons",
    .dev_read = cons_read,
    .dev_write = cons_write,
    .dev_close = cons_close,
    .dev_stat = cons_stat,
};

int iscons(int fdnum) {
	int r;
	struct Fd *fd;

	if ((r = fd_lookup(fdnum, &fd)) < 0) {
		return r;
	}
	return fd->fd_dev_id == devcons.dev_id;
}

int opencons(void) {
	int r;
	struct Fd *fd;

	if ((r = fd_alloc(&fd)) < 0) {
		return r;
	}
	if ((r = syscall_mem_alloc(0, fd, PTE_D | PTE_LIBRARY)) < 0) {
		return r;
	}
	fd->fd_dev_id = devcons.dev_id;
	fd->fd_omode = O_RDWR;
	return fd2num(fd);
}

int cons_read(struct Fd *fd, void *vbuf, u_int n, u_int offset) {
	int c;

	if (n == 0) {
		return 0;
	}

	while ((c = syscall_cgetc()) == 0) {
		syscall_yield();
	}

	if (c != '\r') {
		debugf("%c", c);
	} else {
		debugf("\n");
	}
	if (c < 0) {
		return c;
	}
	if (c == 0x04) { // ctl-d is eof
		return 0;
	}
	*(char *)vbuf = c;
	return 1;
}

int cons_write(struct Fd *fd, const void *buf, u_int n, u_int offset) {
	int r = syscall_print_cons(buf, n);
	if (r < 0) {
		return r;
	}
	return n;
}

int cons_close(struct Fd *fd) {
	return 0;
}

int cons_stat(struct Fd *fd, struct Stat *stat) {
	strcpy(stat->st_name, "<cons>");
	return 0;
}

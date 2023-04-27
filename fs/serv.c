/*
 * File system server main loop -
 * serves IPC requests from other environments.
 */

#include "serv.h"
#include <fd.h>
#include <fsreq.h>
#include <lib.h>
#include <mmu.h>

struct Open {
	struct File *o_file; // mapped descriptor for open file
	u_int o_fileid;	     // file id
	int o_mode;	     // open mode
	struct Filefd *o_ff; // va of filefd page
};

// Max number of open files in the file system at once
#define MAXOPEN 1024
#define FILEVA 0x60000000

// initialize to force into data section
struct Open opentab[MAXOPEN] = {{0, 0, 1}};

// Virtual address at which to receive page mappings containing client requests.
#define REQVA 0x0ffff000

// Overview:
//  Initialize file system server process.
void serve_init(void) {
	int i;
	u_int va;

	// Set virtual address to map.
	va = FILEVA;

	// Initial array opentab.
	for (i = 0; i < MAXOPEN; i++) {
		opentab[i].o_fileid = i;
		opentab[i].o_ff = (struct Filefd *)va;
		va += BY2PG;
	}
}

// Overview:
//  Allocate an open file.
int open_alloc(struct Open **o) {
	int i, r;

	// Find an available open-file table entry
	for (i = 0; i < MAXOPEN; i++) {
		switch (pageref(opentab[i].o_ff)) {
		case 0:
			if ((r = syscall_mem_alloc(0, opentab[i].o_ff, PTE_D | PTE_LIBRARY)) < 0) {
				return r;
			}
		case 1:
			opentab[i].o_fileid += MAXOPEN;
			*o = &opentab[i];
			memset((void *)opentab[i].o_ff, 0, BY2PG);
			return (*o)->o_fileid;
		}
	}

	return -E_MAX_OPEN;
}

// Overview:
//  Look up an open file for envid.
int open_lookup(u_int envid, u_int fileid, struct Open **po) {
	struct Open *o;

	o = &opentab[fileid % MAXOPEN];

	if (pageref(o->o_ff) == 1 || o->o_fileid != fileid) {
		return -E_INVAL;
	}

	*po = o;
	return 0;
}

// Serve requests, sending responses back to envid.
// To send a result back, ipc_send(envid, r, 0, 0).
// To include a page, ipc_send(envid, r, srcva, perm).

void serve_open(u_int envid, struct Fsreq_open *rq) {
	struct File *f;
	struct Filefd *ff;
	int r;
	struct Open *o;

	// Find a file id.
	if ((r = open_alloc(&o)) < 0) {
		ipc_send(envid, r, 0, 0);
	}

	// Open the file.
	if ((r = file_open(rq->req_path, &f)) < 0) {
		ipc_send(envid, r, 0, 0);
		return;
	}

	// Save the file pointer.
	o->o_file = f;

	// Fill out the Filefd structure
	ff = (struct Filefd *)o->o_ff;
	ff->f_file = *f;
	ff->f_fileid = o->o_fileid;
	o->o_mode = rq->req_omode;
	ff->f_fd.fd_omode = o->o_mode;
	ff->f_fd.fd_dev_id = devfile.dev_id;

	ipc_send(envid, 0, o->o_ff, PTE_D | PTE_LIBRARY);
}

void serve_map(u_int envid, struct Fsreq_map *rq) {
	struct Open *pOpen;
	u_int filebno;
	void *blk;
	int r;

	if ((r = open_lookup(envid, rq->req_fileid, &pOpen)) < 0) {
		ipc_send(envid, r, 0, 0);
		return;
	}

	filebno = rq->req_offset / BY2BLK;

	if ((r = file_get_block(pOpen->o_file, filebno, &blk)) < 0) {
		ipc_send(envid, r, 0, 0);
		return;
	}

	ipc_send(envid, 0, blk, PTE_D | PTE_LIBRARY);
}

void serve_set_size(u_int envid, struct Fsreq_set_size *rq) {
	struct Open *pOpen;
	int r;
	if ((r = open_lookup(envid, rq->req_fileid, &pOpen)) < 0) {
		ipc_send(envid, r, 0, 0);
		return;
	}

	if ((r = file_set_size(pOpen->o_file, rq->req_size)) < 0) {
		ipc_send(envid, r, 0, 0);
		return;
	}

	ipc_send(envid, 0, 0, 0);
}

void serve_close(u_int envid, struct Fsreq_close *rq) {
	struct Open *pOpen;

	int r;

	if ((r = open_lookup(envid, rq->req_fileid, &pOpen)) < 0) {
		ipc_send(envid, r, 0, 0);
		return;
	}

	file_close(pOpen->o_file);
	ipc_send(envid, 0, 0, 0);
}

// Overview:
//  Serve to remove a file specified by the path in `req`.
void serve_remove(u_int envid, struct Fsreq_remove *rq) {
	// Step 1: Remove the file specified in 'rq' using 'file_remove' and store its return value.
	int r;
	/* Exercise 5.11: Your code here. (1/2) */

	// Step 2: Respond the return value to the requester 'envid' using 'ipc_send'.
	/* Exercise 5.11: Your code here. (2/2) */

}

void serve_dirty(u_int envid, struct Fsreq_dirty *rq) {
	struct Open *pOpen;
	int r;

	if ((r = open_lookup(envid, rq->req_fileid, &pOpen)) < 0) {
		ipc_send(envid, r, 0, 0);
		return;
	}

	if ((r = file_dirty(pOpen->o_file, rq->req_offset)) < 0) {
		ipc_send(envid, r, 0, 0);
		return;
	}

	ipc_send(envid, 0, 0, 0);
}

void serve_sync(u_int envid) {
	fs_sync();
	ipc_send(envid, 0, 0, 0);
}

void serve(void) {
	u_int req, whom, perm;

	for (;;) {
		perm = 0;

		req = ipc_recv(&whom, (void *)REQVA, &perm);

		// All requests must contain an argument page
		if (!(perm & PTE_V)) {
			debugf("Invalid request from %08x: no argument page\n", whom);
			continue; // just leave it hanging, waiting for the next request.
		}

		switch (req) {
		case FSREQ_OPEN:
			serve_open(whom, (struct Fsreq_open *)REQVA);
			break;

		case FSREQ_MAP:
			serve_map(whom, (struct Fsreq_map *)REQVA);
			break;

		case FSREQ_SET_SIZE:
			serve_set_size(whom, (struct Fsreq_set_size *)REQVA);
			break;

		case FSREQ_CLOSE:
			serve_close(whom, (struct Fsreq_close *)REQVA);
			break;

		case FSREQ_DIRTY:
			serve_dirty(whom, (struct Fsreq_dirty *)REQVA);
			break;

		case FSREQ_REMOVE:
			serve_remove(whom, (struct Fsreq_remove *)REQVA);
			break;

		case FSREQ_SYNC:
			serve_sync(whom);
			break;

		default:
			debugf("Invalid request code %d from %08x\n", whom, req);
			break;
		}

		syscall_mem_unmap(0, (void *)REQVA);
	}
}

int main() {
	user_assert(sizeof(struct File) == BY2FILE);

	debugf("FS is running\n");

	serve_init();
	fs_init();

	serve();
	return 0;
}

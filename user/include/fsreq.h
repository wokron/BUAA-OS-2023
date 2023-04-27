#ifndef _FSREQ_H_
#define _FSREQ_H_

#include <fs.h>
#include <types.h>

// Definitions for requests from clients to file system

#define FSREQ_OPEN 1
#define FSREQ_MAP 2
#define FSREQ_SET_SIZE 3
#define FSREQ_CLOSE 4
#define FSREQ_DIRTY 5
#define FSREQ_REMOVE 6
#define FSREQ_SYNC 7

struct Fsreq_open {
	char req_path[MAXPATHLEN];
	u_int req_omode;
};

struct Fsreq_map {
	int req_fileid;
	u_int req_offset;
};

struct Fsreq_set_size {
	int req_fileid;
	u_int req_size;
};

struct Fsreq_close {
	int req_fileid;
};

struct Fsreq_dirty {
	int req_fileid;
	u_int req_offset;
};

struct Fsreq_remove {
	char req_path[MAXPATHLEN];
};

#endif

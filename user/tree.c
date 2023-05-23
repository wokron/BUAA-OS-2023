#include <lib.h>

int flag[256];

int has_next[256];

void usage(void) {
	printf("usage: tree [-L [n]] [dir]\n");
	exit();
}

void print_file(char *name, int level) {
	for (int i = 1; i < level; i++) {
		printf("%c    ", has_next[i] ? '|' : ' ');
	}
		printf("|--- ");
	printf("%s\n", name);
}

void dfs_walk_path(char *path, int level) {
	int fd, n;
	struct File f, f_next;
	struct File files[256];
	int file_num = 0;
	
	has_next[level] = 1;

	if ((fd = open(path, O_RDONLY)) < 0) {
		user_panic("open %s, %d", path, fd);
	}

	while ((n = readn(fd, &f, sizeof f)) == sizeof f) {
		if (f.f_name[0]) {
			files[file_num++] = f;
		}
	}
	
	int len = strlen(path);
	path[len++] = '/';

	for (int i = 0; i < file_num; i++) {
		f = files[i];
		if (i == file_num - 1) {
			has_next[level] = 0;
		}
		print_file(f.f_name, level);
		if (f.f_type == FTYPE_DIR) {
			strcpy(path + len, f.f_name);
			dfs_walk_path(path, level + 1);
			path[len] = '\0';
		}
	}

	close(fd);
//	if (n > 0) {
//		user_panic("short read in directory %s", path);
//	}
//	if (n < 0) {
//		user_panic("error reading directory %s: %d", path, n);
//	}

}

int main(int argc, char *argv[]) {
	int r;

	ARGBEGIN {
	default:
		usage();
	case 'L':
		flag[(u_char)ARGC()]++;
		break;
	}
	ARGEND	

	if (argc != 1) {
		usage();
	}
	
	char path_buf[MAXPATHLEN];
	strcpy(path_buf, argv[0]);

	dfs_walk_path(path_buf, 1);

	return 0;
}

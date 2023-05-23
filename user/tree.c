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

	if (flag['L'] > 0 && level > flag['L']) {
		return;
	}
	
	has_next[level] = 1;

	if ((fd = open(path, O_RDONLY)) < 0) {
		user_panic("open %s, %d", path, fd);
	}

	while ((n = readn(fd, &f, sizeof f)) == sizeof f) {
		if (f.f_name[0]) {
			files[file_num++] = f;
		}
	}
	
	if (n > 0) {
		user_panic("short read in directory %s", path);
	}
	if (n < 0) {
		user_panic("error reading directory %s: %d", path, n);
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
}

int atoi(char *a) {
	int num = 0;
	for (int i = 0; a[i]; i++) {
		num *= 10;
		num += a[i] - '0';
	}
	return num;
}

int main(int argc, char *argv[]) {
	int r;

	ARGBEGIN {
	default:
		usage();
	case 'L':
		flag[(u_char)ARGC()] = atoi(ARGF());
		break;
	}
	ARGEND	

	if (argc != 1) {
		usage();
	}
	
	char path_buf[MAXPATHLEN];
	strcpy(path_buf, argv[0]);
	
	char *name = path_buf;
	for (int i = 0; path_buf[i]; i++) {
		if (path_buf[i] == '/')
			name = path_buf + i;
	}

	struct Stat stat_buf;
	stat(path_buf, &stat_buf);

	if (!stat_buf.st_isdir) {
		user_panic("%s is not directory", path_buf);
	}

	printf("%s\n", name);
	dfs_walk_path(path_buf, 1);

	return 0;
}

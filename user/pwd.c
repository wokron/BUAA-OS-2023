#include <lib.h>

int main() {
	char path[MAXPATHLEN];
	getcwd(path);
	printf("%s\n", path);
	return 0;
}

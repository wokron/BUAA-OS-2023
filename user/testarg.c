#include <lib.h>

int main(int argc, char **argv) {
	int i;
	for (i = 0; i < argc; i++) {
		debugf("'''''''' %s '''''''''\n", argv[i]);
	}
	return 0;
}

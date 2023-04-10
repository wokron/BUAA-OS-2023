#include <lib.h>

int main() {
	int a = 0;
	int id = 0;

	if ((id = fork()) == 0) {
		if ((id = fork()) == 0) {
			a += 3;

			for (;;) {
				debugf("\t\tthis is child2 :a:%d\n", a);
			}
		}

		a += 2;

		for (;;) {
			debugf("\tthis is child :a:%d\n", a);
		}
	}

	a++;

	for (;;) {
		debugf("this is father: a:%d\n", a);
	}
	return 0;
}

#include <lib.h>

int main() {
	int a = 0;
	int id = 0;
	debugf("fktest on ostest!\n");
	if ((id = fork()) == 0) {
		if ((id = fork()) == 0) {
			if ((id = fork()) == 0) {
				if ((id = fork()) == 0) {
					a += 5;
					int iiiii;
					for (iiiii = 0; iiiii < 50; iiiii++) {
						debugf("\t\t\t\t@this is child4 :a:%d\n", a);
						syscall_yield();
					}
					return 0;
				}
				a += 4;
				int iiii;
				for (iiii = 0; iiii < 50; iiii++) {
					debugf("\t\t\t@this is child3 :a:%d\n", a);
					syscall_yield();
				}
				return 0;
			}
			a += 3;
			int i;
			for (i = 0; i < 50; i++) {
				debugf("\t\t@this is child2 :a:%d\n", a);
				syscall_yield();
			}
			return 0;
		}
		a += 2;
		int ii;
		for (ii = 0; ii < 50; ii++) {
			debugf("\t@this is child1 :a:%d\n", a);
			syscall_yield();
		}
		return 0;
	}
	a++;
	int iii;
	for (iii = 0; iii < 50; iii++) {
		debugf("@this is father: a:%d\n", a);
		syscall_yield();
	}
	debugf("@father exit\n");
	return 0;
}

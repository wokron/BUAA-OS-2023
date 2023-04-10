#include <lib.h>

int main() {
	int a = 0;
	if (fork() == 0) {
		if (fork() == 0 && fork() == 0 && fork() == 0 && fork() == 0 && fork() == 0) {
			debugf("!@this is child %x :a:%d\n", syscall_getenvid(), a);
		}
		return 0;
	}
	debugf("!@ancestor exit\n");
	return 0;
}

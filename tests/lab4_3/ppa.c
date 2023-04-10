#include <lib.h>
const int PP1 = 0x800, PP2 = 0x1001;

int main() {
	u_int me, who, i;
	me = syscall_getenvid();
	debugf("i am %x\n", me);
	if (me == PP1) {
		who = PP2;
		debugf("\n@@@@@send 0 from %x to %x\n", me, who);
		ipc_send(who, 0, 0, 0);
	} else if (me == PP2) {
		who = PP1;
	} else {
		debugf("unexpected envid %x\n", me);
		syscall_panic("halt");
	}
	for (;;) {
		debugf("%x am waiting.....\n", me);
		i = ipc_recv(0, 0, 0);
		debugf("%x got %d from %x\n", me, i, who);
		if (i == 10) {
			user_panic("%x stop", me);
		}
		i++;
		debugf("\n@@@@@send %d from %x to %x\n", i, me, who);
		ipc_send(who, i, 0, 0);
		if (i == 10) {
			user_panic("%x stop", me);
		}
	}
	return 0;
}

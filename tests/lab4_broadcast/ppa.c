#include <lib.h>
const int PP1 = 0x800, PP2 = 0x1001, PP3 = 0x1802, PP4 = 0x2003;
int main() {
	u_int me, who, i;
	me = syscall_getenvid();
	debugf("%x father is %x\n", env->env_id, env->env_parent_id);
	if (me == PP1) {
		char str[40] = "hello world!~";
		char *ptr = (char *)UTEMP;
		strcpy(ptr, str);
		debugf("\n@@@@@%x: is broadcasting\n", me);
		ipc_broadcast(111, ptr, PTE_V);

	} else if (me == PP2 || me == PP3 || me == PP4) {
		;
	} else {
		debugf("unexpected envid %x\n", me);
		syscall_panic("halt");
	}

	if (me != PP1) {
		char *ptr = (char *)UTEMP;
		debugf("\n@@@@@%x: is recving.....\n", me);
		i = ipc_recv(&who, ptr, 0);
		debugf("\n@@@@@%x recved from %x: [message]\"%s\", [value]%d\n", me, who, ptr, i);
	}

	for (;;) {
	}
	return 0;
}

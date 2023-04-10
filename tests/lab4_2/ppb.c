#include <lib.h>

// envid is deterministic, since all Envs are created in kernel:init.c before trap_init
const int PPC = 0x1001;

int main() {
	int me, r;
	me = syscall_getenvid();
	debugf("i am %x\n", me);
	r = syscall_mem_alloc(me, (void *)UTOP, 0); // va illegal
	if (r < 0) {
		debugf("syscall_mem_alloc_ok_1\n");
	} else {
		user_halt("syscall_mem_alloc\n");
	}
	r = syscall_mem_alloc(me, NULL, 0); // va illegal
	if (r < 0) {
		debugf("syscall_mem_alloc_ok_2\n");
	} else {
		user_halt("syscall_mem_alloc\n");
	}
	r = syscall_mem_alloc(me, (void *)(UTEMP - BY2PG), 0); // va illegal
	if (r < 0) {
		debugf("syscall_mem_alloc_ok_3\n");
	} else {
		user_halt("syscall_mem_alloc\n");
	}
	r = syscall_mem_alloc(me, (void *)KERNBASE, 0); // va illegal
	if (r < 0) {
		debugf("syscall_mem_alloc_ok_4\n");
	} else {
		user_halt("syscall_mem_alloc\n");
	}
	r = syscall_mem_alloc(0x3000, (void *)UTEMP, 0); // no exist envid
	if (r < 0) {
		debugf("syscall_mem_alloc_ok_5\n");
	} else {
		user_halt("syscall_mem_alloc\n");
	}
	r = syscall_mem_alloc(me, (void *)UTEMP, 0);
	if (r == 0) {
		debugf("syscall_mem_alloc_ok_6\n");
	} else {
		user_halt("syscall_mem_alloc\n");
	}

	r = syscall_mem_map(me, (void *)UTOP, PPC, (void *)UTEMP, 0); // src >= UTOP
	if (r < 0) {
		debugf("syscall_mem_map_ok_1\n");
	} else {
		user_halt("syscall_mem_map\n");
	}
	r = syscall_mem_map(me, (void *)UTEMP, PPC, (void *)UTOP, 0); // dst >= UTOP
	if (r < 0) {
		debugf("syscall_mem_map_ok_2\n");
	} else {
		user_halt("syscall_mem_map\n");
	}
	r = syscall_mem_map(me, (void *)UTEMP, PPC, (void *)KERNBASE, 0); // dstva illegal
	if (r < 0) {
		debugf("syscall_mem_map_ok_3\n");
	} else {
		user_halt("syscall_mem_map\n");
	}
	r = syscall_mem_map(me, (void *)UTEMP, 0x3000, (void *)UTEMP, 0); // no exist envid
	if (r < 0) {
		debugf("syscall_mem_map_ok_4\n");
	} else {
		user_halt("syscall_mem_map\n");
	}
	r = syscall_mem_map(me, (void *)UTEMP, PPC, (void *)UTEMP, 0);
	if (r == 0) {
		debugf("syscall_mem_map_ok_5\n");
	} else {
		user_halt("syscall_mem_map: %d\n", r);
	}

	r = syscall_mem_unmap(me, (void *)(UTOP + BY2PG)); // va >= UTOP
	if (r < 0) {
		debugf("syscall_mem_unmap_ok_1\n");
	} else {
		user_halt("syscall_mem_unmap\n");
	}
	r = syscall_mem_unmap(0x3000, (void *)UTEMP); // no exist envid
	if (r < 0) {
		debugf("syscall_mem_unmap_ok_2\n");
	} else {
		user_halt("syscall_mem_unmap\n");
	}
	r = syscall_mem_unmap(me, (void *)UTEMP);
	if (r == 0) {
		debugf("syscall_mem_unmap_ok_3\n");
	} else {
		user_halt("syscall_mem_unmap\n");
	}
	r = syscall_mem_unmap(me, (void *)UTEMP);
	if (r == 0) {
		debugf("syscall_mem_unmap_ok_4\n");
	} else {
		user_halt("syscall_mem_unmap\n");
	}

	user_halt("test passed\n");
	return 0;
}

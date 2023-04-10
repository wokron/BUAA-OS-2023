#include "ppa.h"

volatile u_int *timer = (u_int *)UTEMP;

void sleep(u_int t) {
	t += *timer;
	while (*timer < t) {
		syscall_yield();
	}
}

void sheep() {
	int i;
	for (i = 0;; ++i) {
		syscall_yield();
		if (i >= 0xabc) {
			++*timer;
			i = 0;
		}
	}
}

void srv(u_int pvt) {
	u_int me = env->env_id;
	debugf("%x: srv up, pvt is %x\n", me, pvt);

	sleep(1);

	int i;
	u_int *buf = (u_int *)0x60000000;
	syscall_mem_alloc(0, buf, PTE_D);
	for (i = 0; i < (tot >> 1); ++i) {
		u_int who = 0;
		u_int v = ipc_recv(&who, 0, 0);
		debugf("%x: got %x from %x\n", me, v, who);
		uassert(v == who + me);
		buf[i << 1] = who;
		buf[i << 1 | 1] = v;
	}

	sleep(1);

	if (me == pvt) {
		i = 0;
		for (i = 0; i < (tot >> 1); ++i) {
			ipc_send(buf[i << 1], buf[i << 1 | 1] + me, 0, 0);
		}
		syscall_mem_unmap(0, buf);
		while (1) {
			buf += BY2PG >> 2; // avoiding tlb issues
			u_int who = 0, perm = 0;
			u_int n = ipc_recv(&who, buf, &perm);
			uassert((perm & PTE_V));
			uassert(n == (tot >> 1));
			int i;
			for (i = 0; i < n; ++i) {
				u_int dst = buf[i << 1];
				u_int v = buf[i << 1 | 1];
				uassert(v == who + dst);
				ipc_send(dst, v - who + me + me, 0, 0);
			}
		}
	} else {
		ipc_send(pvt, tot >> 1, buf, PTE_V);
	}
}

void cli(u_int pvt, u_int *srvs) {
	u_int me = env->env_id;
	debugf("%x: cli up, pvt is %x, pvt2 is %x\n", me, pvt, *srvs);
	int i;

	for (i = 0; i < (tot >> 1); ++i) {
		u_int dst = srvs[i];
		debugf("%x: sending %x to %x\n", me, dst + me, dst);
		ipc_send(dst, me + dst, 0, 0);
	}

	if (me == pvt) {
		i += (tot >> 1) - 1;
	}

	while (i--) {
		u_int who = 0;
		u_int v = ipc_recv(&who, 0, 0);
		debugf("%x: got %x from %x\n", me, v, who);
		uassert(who == srvs[0] || me == pvt);
		uassert(v == me + who + who);
	}

	if (me == pvt) {
		accepted();
	} else {
		ipc_send(pvt, me + me + pvt, 0, 0);
	}
}

int fork_n(int n) {
	if (--n <= 0) {
		return 0;
	}
	int r = fork();
	uassert(r >= 0);
	return r ? n : fork_n(n);
}

int main() {
	syscall_mem_alloc(0, (void *)timer, PTE_D | PTE_LIBRARY);
	*timer = 0;
	if (fork_n(2)) {
		sheep();
	}

	u_int pvt = env->env_id;

	volatile u_int *shm = (u_int *)0x1000000;
	syscall_mem_alloc(0, (void *)shm, PTE_D | PTE_LIBRARY);

	volatile u_int *srvs = shm + 1000;
	int i;
	for (i = 0; i < (tot >> 1); ++i) {
		srvs[i] = 0;
	}

	i = fork_n(1 + (tot >> 1));
	if (i < (tot >> 1)) {
		srvs[i] = env->env_id;
		while (!srvs[0]) {
			syscall_yield();
		}
		srv(srvs[0]);
		exit();
	}

	fork_n(tot >> 1);
	for (i = 0; i < (tot >> 1); ++i) {
		while (!srvs[i]) {
			syscall_yield();
		}
	}

	cli(pvt, (u_int *)srvs);
	return 0;
}

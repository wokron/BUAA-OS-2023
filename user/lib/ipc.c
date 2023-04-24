// User-level IPC library routines

#include <env.h>
#include <lib.h>
#include <mmu.h>

// Send val to whom.  This function keeps trying until
// it succeeds.  It should panic() on any error other than
// -E_IPC_NOT_RECV.
//
// Hint: use syscall_yield() to be CPU-friendly.
void ipc_send(u_int whom, u_int val, const void *srcva, u_int perm) {
	int r;
	while ((r = syscall_ipc_try_send(whom, val, srcva, perm)) == -E_IPC_NOT_RECV) {
		syscall_yield();
	}
	user_assert(r == 0);
}

// Receive a value.  Return the value and store the caller's envid
// in *whom.
//
// Hint: use env to discover the value and who sent it.
u_int ipc_recv(u_int *whom, void *dstva, u_int *perm) {
	int r = syscall_ipc_recv(dstva);
	if (r != 0) {
		user_panic("syscall_ipc_recv err: %d", r);
	}

	if (whom) {
		*whom = env->env_ipc_from;
	}

	if (perm) {
		*perm = env->env_ipc_perm;
	}

	return env->env_ipc_value;
}

int is_father(u_int may_child, u_int may_father) {
	if (may_child == may_father)
		return 0;
	while (may_child != 0) {
		if (may_child == may_father)
			return 1;
		struct Env * e = envs + ENVX(may_child);
		may_child = e->env_parent_id;
	}
	return 0;
}


void ipc_broadcast(u_int val, void * srcva, u_int perm) {
	struct Env * childs[NENV];
	int child_num = 0;
	for (int i = 0; i < NENV; i++) {
		if (is_father(envs[i].env_id, env->env_id)) {
			childs[child_num++] = envs + i;
		}
	}

	int r;
	for (int i = 0; i < child_num; i++) {
		while ((r = syscall_ipc_try_send(childs[i]->env_id, val, srcva, perm)) != 0) {
			if (r == -E_IPC_NOT_RECV)
				syscall_yield();
		}
	}
}

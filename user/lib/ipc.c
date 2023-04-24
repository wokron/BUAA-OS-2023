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


/* copy to user/lib/ipc.c */

void set_gid(u_int gid) {
    // 你需要实现此 syscall_set_gid 系统调用
    syscall_set_gid(gid);
}

int ipc_group_send(u_int whom, u_int val, const void *srcva, u_int perm) {
    int r;
    // 你需要实现此 syscall_ipc_try_group_send 系统调用
    while ((r = syscall_ipc_try_group_send(whom, val, srcva, perm)) != 0) {
        // 接受方进程尚未准备好接受消息，进程切换，后续继续轮询尝试发送请求
        if (r == -E_IPC_NOT_RECV) syscall_yield();
        // 接收方进程准备好接收消息，但非同组通信，消息发送失败，停止轮询，返回错误码 -E_IPC_NOT_GROUP
        if (r == -E_IPC_NOT_GROUP) return -E_IPC_NOT_GROUP;
    }
    // 函数返回0，告知用户成功发送消息
    return 0;
}

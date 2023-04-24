#include <lib.h>
const int PP1 = 0x800, PP2 = 0x1001;
int main() {
	u_int me, who;
	u_int wrong_message = -1;
	u_int correct_message = 1;

	me = syscall_getenvid();
	debugf("i am %x\n", me);
	if (me == PP1) {
		who = PP2;

		set_gid(11);
		int group_send_error = ipc_group_send(who, wrong_message, 0, 0);
		debugf("\ngroup_send_error should be -E_IPC_NOT_GROUP -14: you got %d\n",
		       group_send_error);
		user_assert(group_send_error == -E_IPC_NOT_GROUP);

		set_gid(0);
		int group_send_success = ipc_group_send(who, correct_message, 0, 0);
		debugf("\ngroup_send_success should be 0: you got %d\n", group_send_success);
		user_assert(group_send_success == 0);

		debugf("\n@@@@@send %d from %x to %x\n", correct_message, me, who);

	} else if (me == PP2) {
		who = PP1;
		u_int receive_message = ipc_recv(0, 0, 0);
		debugf("\n%x got %d from %x\n", me, receive_message, who);

		user_assert(receive_message == correct_message);

	} else {
		syscall_panic("halt");
	}

	debugf("\n%x successfully end\n", me);
	return 0;
}

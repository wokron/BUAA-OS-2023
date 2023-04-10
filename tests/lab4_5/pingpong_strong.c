// Ping-pong a counter between two processes.
// Only need to start one of these -- splits into two with fork.

#include <lib.h>

static int A[1024];
static int B[1024];
static int C[1024];
static int D[1024];

int main() {
	u_int who, i;
	int isSon = 0;
	int *ptr;
	int father;
	u_int perm;

	memset(A, 0, BY2PG);
	memset(B, 0, BY2PG);
	memset(C, 0, BY2PG);
	memset(D, 0, BY2PG);
	father = syscall_getenvid();

	if ((who = fork()) != 0) {
		// get the ball rolling
		debugf("@@@@@send 0 from %x to %x\n", syscall_getenvid(), who);
		ipc_send(who, 0, 0, 0);
		// user_panic("&&&&&&&&&&&&&&&&&&&&&&&&m");
	} else {
		isSon = 1;
	}

	for (;;) {
		debugf("@@@@%x am waiting.....\n", syscall_getenvid());
		i = ipc_recv(&who, 0, 0);

		debugf("@@@@%x got %d from %x\n", syscall_getenvid(), i, who);

		// user_panic("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&");
		if (i == 10) {
			break;
		}

		i++;
		debugf("@@@@@send %d from %x to %x\n", i, syscall_getenvid(), who);
		ipc_send(who, i, 0, 0);

		if (i == 10) {
			break;
		}
	}

	debugf("@@@@ %x finish test 1\n", syscall_getenvid());

	//--------------------------------------------------------------------------------

	// int *ptr = ROUNDWON(B, BY2PG);
	// ptr = ROUNDDOWN(B, BY2PG);

	if (isSon) {
		if ((who = fork()) == 0) {
			// grandSon
			debugf("@@@@grandson %x waiting for son's envid\n", syscall_getenvid());
			i = ipc_recv(&who, 0, 0);

			debugf("@@@@grandson %x got envid %x from son %x\n", syscall_getenvid(), i,
			       who);

			ptr = (int *)ROUNDDOWN(C, BY2PG);
			ptr[0] = 2020;
			ptr[100] = syscall_getenvid();
			ptr[200] = i;
			ptr[300] = 4;
			debugf("@@@@grandson %x send Page to son %x\n", syscall_getenvid(), i);
			ipc_send(i, 0, ptr, PTE_D | PTE_LIBRARY);

			debugf("@@@@grandson %x is waiting ack from son %x\n", syscall_getenvid(),
			       i);
			i = ipc_recv(&who, 0, 0);
			if (i != 0) {
				user_panic("Error when grandson get ack from son!");
			}
			// debugf("@@@@grandson %x got ack from son %x\n", syscall_getenvid(),
			// who);

			debugf("@@@@2 %d lab %d strong testcase2\n", ptr[300], ptr[0]);
			debugf("@@@@2 send is %x recv is %x\n", ptr[100], ptr[200]);
			debugf("@@@@grandson %x send continue to son %x\n", syscall_getenvid(),
			       who);
			ipc_send(who, 0, 0, 0);

			debugf("@@@@grandson %x wait for father signal\n", syscall_getenvid());
			i = ipc_recv(&who, 0, 0);
			if (i != 0) {
				user_panic("Error when grandson get ack from father!");
			}
			debugf("@@@@4 %d lab %d strong testcase4\n", ptr[100], ptr[200]);
			debugf("@@@@4 send is %x recv is %x\n", ptr[0], ptr[300]);

		} else {
			// Son
			debugf("@@@@son tell grandson his envid %x\n", syscall_getenvid());
			ipc_send(who, syscall_getenvid(), 0, 0);

			debugf("@@@@son %x is waiting from grandSon %x page\n", syscall_getenvid(),
			       who);

			ptr = (int *)ROUNDDOWN(B, BY2PG);
			i = ipc_recv(&who, ptr, &perm);
			if (i != 0) {
				user_panic("Error when son get Page from grandson!");
			}
			debugf("@@@@1 %d lab %d strong testcase1\n", ptr[0], ptr[300]);
			debugf("@@@@1 send is %x recv is %x\n", ptr[100], ptr[200]);

			ptr[0] = 4;
			ptr[100] = syscall_getenvid();
			ptr[200] = who;
			ptr[300] = 2020;
			debugf("@@@@son %x send ack to grandson %x\n", syscall_getenvid(), who);
			ipc_send(who, 0, 0, 0);
			debugf("@@@@son %x wait continue from grandson %x\n", syscall_getenvid(),
			       who);
			i = ipc_recv(&who, 0, 0);
			if (i != 0) {
				user_panic("Error when son get continue from grandson");
			}

			debugf("@@@@son %x send Page to father %x\n", syscall_getenvid(), father);
			ipc_send(father, 0, ptr, perm);
			debugf("@@@@son %x tell father grandson id %x\n", syscall_getenvid(), who);
			ipc_send(father, who, 0, 0);
			debugf("@@@@son %x wait for father %x signal\n", syscall_getenvid(),
			       father);
			i = ipc_recv(&who, 0, 0);
			if (i != 0) {
				user_panic("Error when wait for father signal!");
			}
			debugf("@@@@3 %d lab %d strong testcase3\n", ptr[100], ptr[200]);
			debugf("@@@@3 send is %x recv is %x\n", ptr[0], ptr[300]);
		}
	} else {
		// father
		ptr = (int *)ROUNDDOWN(D, BY2PG);
		debugf("@@@@father %x wait for son page\n", syscall_getenvid());
		i = ipc_recv(&who, ptr, &perm);
		if (i != 0) {
			user_panic("Error when father get Page from son");
		}
		debugf("@@@@father %x wait for grandson id\n", syscall_getenvid());
		i = ipc_recv(&who, 0, 0);
		// if (i != 0) {
		//	user_panic("Error when father wait for grandson id");
		// }
		debugf("@@@@father %x got grandson id %x from son %x\n", syscall_getenvid(), i,
		       who);
		ptr[100] = 2020;
		ptr[200] = 4;
		ptr[0] = syscall_getenvid();
		ptr[300] = who;
		ptr[300] = i;
		// The answer was provided as if i is written before son finishes recv, so we do it
		// earlier to avoid sched racing. To enhance the case, we may modify ptr[300] after
		// the son sends an ack.
		debugf("@@@@father send signal to son\n");
		ipc_send(who, 0, 0, 0);
		debugf("@@@@father send signal to grandson\n");
		ipc_send(i, 0, 0, 0);
	}
	debugf("@@@@I am killing %x\n", syscall_getenvid());

	return 0;
}

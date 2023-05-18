#include <lib.h>

char *msg = "Now is the time for all good men to come to the aid of their party.";

int main() {
	char buf[100];
	int i, pid, p[2];

	if ((i = pipe(p)) < 0) {
		user_panic("pipe: %d", i);
	}

	if ((pid = fork()) < 0) {
		user_panic("fork: %d", i);
	}

	if (pid == 0) {
		debugf("[%08x] pipe_readeof close %d\n", env->env_id, p[1]);
		close(p[1]);
		debugf("[%08x] pipe_readeof readn %d\n", env->env_id, p[0]);
		i = readn(p[0], buf, sizeof buf - 1);
		if (i < 0) {
			user_panic("read: %d", i);
		}
		buf[i] = 0;
		if (strcmp(buf, msg) == 0) {
			debugf("\npipe read closed properly\n");
		} else {
			debugf("\ngot %d bytes: %s\n", i, buf);
		}
		exit();
	} else {
		debugf("[%08x] pipe_readeof close %d\n", env->env_id, p[0]);
		close(p[0]);
		debugf("[%08x] pipe_readeof write %d\n", env->env_id, p[1]);
		if ((i = write(p[1], msg, strlen(msg))) != strlen(msg)) {
			user_panic("write: %d", i);
		}
		close(p[1]);
	}
	wait(pid);

	if ((i = pipe(p)) < 0) {
		user_panic("pipe: %d", i);
	}

	if ((pid = fork()) < 0) {
		user_panic("fork: %d", i);
	}

	if (pid == 0) {
		close(p[0]);
		for (;;) {
			debugf(".");
			if (write(p[1], "x", 1) != 1) {
				break;
			}
		}
		debugf("\npipe write closed properly\n");
	}
	close(p[0]);
	close(p[1]);
	wait(pid);

	debugf("pipe tests passed\n");
	return 0;
}

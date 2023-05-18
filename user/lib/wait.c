#include <env.h>
#include <lib.h>
void wait(u_int envid) {
	volatile struct Env *e;

	e = &envs[ENVX(envid)];
	while (e->env_id == envid && e->env_status != ENV_FREE) {
		syscall_yield();
	}
}

#include <env.h>
#include <print.h>
#include <printk.h>

void outputk(void *data, const char *buf, size_t len);

void _panic(const char *file, int line, const char *func, const char *fmt, ...) {
	u_long sp, ra, badva, sr, cause, epc;
	asm("move %0, $29" : "=r"(sp) :);
	asm("move %0, $31" : "=r"(ra) :);
	asm("mfc0 %0, $8" : "=r"(badva) :);
	asm("mfc0 %0, $12" : "=r"(sr) :);
	asm("mfc0 %0, $13" : "=r"(cause) :);
	asm("mfc0 %0, $14" : "=r"(epc) :);

	printk("panic at %s:%d (%s): ", file, line, func);

	va_list ap;
	va_start(ap, fmt);
	vprintfmt(outputk, NULL, fmt, ap);
	va_end(ap);

	printk("\n"
	       "ra:    %08x  sp:  %08x  Status: %08x\n"
	       "Cause: %08x  EPC: %08x  BadVA:  %08x\n",
	       ra, sp, sr, cause, epc, badva);

#if !defined(LAB) || LAB >= 3
	extern struct Env envs[];
	extern struct Env *curenv;
	extern struct Pde *cur_pgdir;

	if ((u_long)curenv >= KERNBASE) {
		printk("curenv:    %x (id = 0x%x, off = %d)\n", curenv, curenv->env_id,
		       curenv - envs);
	} else if (curenv) {
		printk("curenv:    %x (invalid)\n", curenv);
	} else {
		printk("curenv:    NULL\n");
	}

	if ((u_long)cur_pgdir >= KERNBASE) {
		printk("cur_pgdir: %x\n", cur_pgdir);
	} else if (cur_pgdir) {
		printk("cur_pgdir: %x (invalid)\n", cur_pgdir);
	} else {
		printk("cur_pgdir: NULL\n", cur_pgdir);
	}
#endif

#ifdef MOS_HANG_ON_PANIC
	while (1) {
	}
#else
	halt();
#endif
}

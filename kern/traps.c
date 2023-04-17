#include <env.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>

extern void handle_int(void);
extern void handle_tlb(void);
extern void handle_sys(void);
extern void handle_mod(void);
extern void handle_reserved(void);
extern void handle_ov(void);

void (*exception_handlers[32])(void) = {
    [0 ... 31] = handle_reserved,
    [0] = handle_int,
    [2 ... 3] = handle_tlb,
    [12] = handle_ov,
#if !defined(LAB) || LAB >= 4
    [1] = handle_mod,
    [8] = handle_sys,
#endif
};

/* Overview:
 *   The fallback handler when an unknown exception code is encountered.
 *   'genex.S' wraps this function in 'handle_reserved'.
 */
void do_reserved(struct Trapframe *tf) {
	print_tf(tf);
	panic("Unknown ExcCode %2d", (tf->cp0_cause >> 2) & 0x1f);
}

void do_ov(struct Trapframe *tf) {
	// 你需要在此处实现问题描述的处理要求
	curenv->env_ov_cnt++;
	u_long addr = tf->cp0_epc;
	struct Page * pp = page_lookup(curenv->env_pgdir, addr, NULL);
	u_long kva = page2kva(pp) | (addr & 0xfff);

	u_int inst = *((u_int *)kva);
	if ((inst >> 28) != 0) {
		printk("addi ov handled\n");
		u_int imm = inst & 0xffff;
		u_int regsx = (inst >> 21) & ((1 << 5)-1);
		u_int regtx = (inst >> 16) & ((1 << 5)-1);
//		printk("s:%x; t:%x, imm:%x", tf->regs[regsx], tf->regs[regtx], imm);
		tf->regs[regtx] = tf->regs[regsx] / 2 + imm / 2;
	} else {
		if ((inst & ((1<<2)-1)) == 0) {
			printk("add ov handled\n");
			inst |= 1;
		} else {
			printk("sub ov handled\n");
			inst |= 1;
		}
		*((u_int *)kva) = inst;
	}
}

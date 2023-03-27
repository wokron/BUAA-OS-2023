#include <asm/asm.h>
#include <stackframe.h>

.section .text.tlb_miss_entry
tlb_miss_entry:
	j       exc_gen_entry

.section .text.exc_gen_entry
exc_gen_entry:
	SAVE_ALL
/* Exercise 3.9: Your code here. */
	mfc0 	t0, CP0_CAUSE
	andi 	t0, 0x7c
	lw 	t0, exception_handlers(t0)
	jr 	t0


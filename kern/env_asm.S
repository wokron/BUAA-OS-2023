#include <asm/asm.h>
#include <mmu.h>
#include <trap.h>

.text
LEAF(env_pop_tf)
.set reorder
.set at
	sll     a1, a1, 6
	mtc0    a1, CP0_ENTRYHI
	move    sp, a0
	j       ret_from_exception
END(env_pop_tf)

LEAF(enable_irq)
	li      t0, (STATUS_CU0 | STATUS_IM4 | STATUS_IEc)
	mtc0    t0, CP0_STATUS
	jr      ra
END(enable_irq)

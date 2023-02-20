#include <asm/asm.h>

.text
EXPORT(_start)
.set at
.set reorder

	mtc0    zero, CP0_STATUS
	li      sp, 0x80400000
	jal     main
	j       m_halt

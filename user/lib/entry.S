#include <asm/asm.h>

.text
EXPORT(_start)
	lw      a0, 0(sp)
	lw      a1, 4(sp)
	jal     libmain

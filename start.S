#include <asm/asm.h>
.data
.align 2 # align to 4-byte boundary (2^2)
var:
.byte 3 # correctly aligned byte: 3
/* '<x>' in the comments is the part to be replaced. */
/* use '.align <x>' to align the following words to 1-byte boundary (disabling word-alignment) */
/* so that the byte 3 and word 7 is "connected" */
/* Your code here. (1/6) */

.word 7, 8, 9

.text
/* declare a symbol '_start_mips' via the macro 'EXPORT' in 'include/asm/asm.h'. We define '_start_mips' here as the entry of our program. */
/* Your code here. (2/6) */

	mtc0    zero, CP0_STATUS
	li      sp, 0x84000000
	li      a0, 0
	li      a1, 1
	li      a2, 2
	li      a3, 3
	/* use 'addiu  sp, sp, <x>' to push a proper-sized frame onto the stack */
	/* Your code here. (3/6) */

	lw      t1, var
	li      t2, 5
	/* use 'sw  t1, <x>(sp)' to store t1 at the proper place of the stack */
	/* so that t1 is 5th argument of function hello */
	/* Your code here. (4/6) */

	/* use 'sw  t2, <x>(sp)' to store t2 at the proper place of the stack */
	/* so that t2 is 6th argument of function hello */
	/* Your code here. (5/6) */

	/* use 'j' to call the function 'hello', we use 'j' instead of 'jal' because 'hello' is 'noreturn' */
	/* Your code here. (6/6) */

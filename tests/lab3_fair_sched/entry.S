#include <asm/asm.h>

.text
.globl _start
_start:
	jal     main
	j		finish
.org 0x180
finish:
	j       finish

#include <asm/asm.h>

.text
.globl _start
_start:
	jal     main
.org 0x180
finish:
	j       finish

#include <asm/asm.h>

.text
.globl _start
_start:
.extern main
	jal     main

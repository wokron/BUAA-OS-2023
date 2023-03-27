# This user-space program is the same as in tests/lab3_3, which can be built
# with `make test lab=3_3`.
#
# To test this manually:
# - Add the following two lines in init/init.c:
#	ENV_CREATE_PRIORITY(user_bare_loop, 1);
#	ENV_CREATE_PRIORITY(user_bare_loop, 2);
# - Then build the kernel with `CFLAGS=-DMOS_SCHED_MAX_TICKS=100 make`

.text
.globl _start
_start:
	j       _start

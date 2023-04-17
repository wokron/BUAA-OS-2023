CFLAGS    += -DMOS_SCHED_END_PC=0x400180
CFLAGS	  += -DMOS_SCHED_MAX_TICKS=10000
init-override := $(test_dir)/init.c

lab-ge = $(shell [ "$$(echo $(lab)_ | cut -f1 -d_)" -ge $(1) ] && echo true)

targets             := console.o printk.o panic.o

ifeq ($(call lab-ge,2), true)
	targets     += pmap.o tlb_asm.o tlbex.o
endif

ifeq ($(call lab-ge,3), true)
	targets     += env.o env_asm.o sched.o entry.o genex.o kclock.o traps.o
endif

ifeq ($(call lab-ge,4), true)
	targets     += syscall_all.o
endif

CROSS_COMPILE           := mips-linux-gnu-
CC                      := $(CROSS_COMPILE)gcc
CFLAGS                  += --std=gnu99 -EL -G 0 -mno-abicalls -fno-pic -ffreestanding -fno-stack-protector -fno-builtin -Wa,-xgot -Wall -mxgot -mfp32 -march=r3000
LD                      := $(CROSS_COMPILE)ld
LDFLAGS                 += -EL -G 0 -static -n -nostdlib --fatal-warnings
INCLUDES                := -I./include/
objects                 := hello.o output.o start.o
gxemul_files            += hello
gxemul_flags            += -T -C R3000 -M 64

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

.PHONY: all clean dbg

all:    hello

hello:  linker.lds $(objects)
	$(LD) $(LDFLAGS) -o hello -N -T linker.lds $(objects)

clean:
	rm -rf *~ *.o hello *.objdump

dbg:    gxemul_flags    += -V
dbg:    run

run:    gxemul_flags    += -q -E $(shell gxemul -H | grep -q oldtestmips && echo old)testmips

run:
	gxemul $(gxemul_flags) $(gxemul_files)

objdump:
	$(CROSS_COMPILE)objdump hello -aldS > hello.objdump

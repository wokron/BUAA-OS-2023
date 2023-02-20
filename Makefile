ENDIAN         := EL
CROSS_COMPILE  := mips-linux-gnu-
CC             := $(CROSS_COMPILE)gcc
CFLAGS         += --std=gnu99 -$(ENDIAN) -G 0 -mno-abicalls -fno-pic -ffreestanding -fno-stack-protector -fno-builtin -Wa,-xgot -Wall -mxgot -mfp32 -march=r3000
LD             := $(CROSS_COMPILE)ld
LDFLAGS        += -$(ENDIAN) -G 0 -static -n -nostdlib --fatal-warnings
INCLUDES       := -I./include

gxemul_flags   += -T         # halt on non-existant memory accesses
gxemul_flags   += -C R3000   # try to emulate a mips r3000 CPU.
gxemul_flags   += -M 64      # emulate 64 MBs of physical RAM
gxemul_flags   += -E testmips# try to emulate machine type testmips

gxemul_files   += $(target)

link_script    := linker.lds
target         := runbin

objs           := blib.o machine.o start.o test.o

all: $(objs)
	$(LD) $(LDFLAGS) -o $(target) -N -T $(link_script) $(objs)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

dbg: gxemul_flags += -V
dbg: run

run:
	gxemul $(gxemul_flags) $(gxemul_files)

clean:
	rm -f *.o $(target)

.PHONY: all clean run


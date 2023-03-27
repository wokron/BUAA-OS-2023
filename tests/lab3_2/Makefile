targets     := icode_check.b icode_check.x
root_dir    := ../..
tools_dir   := $(root_dir)/tools
INCLUDES    := -I$(root_dir)/include

%.x: %.b.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.b.c: %.b
	$(tools_dir)/bintoc -f $< -o $@ -p test

%.b: entry.o %.o
	$(LD) -o $@ $(LDFLAGS) -T icode_check.lds $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

.PHONY: all clean

all: $(targets)
	cat elf-test-common.c > init.c
	python3 elf-test-gen.py icode_check.b >> init.c

clean:
	rm -rf *.o *~ *.x *.b *.b.c init.c

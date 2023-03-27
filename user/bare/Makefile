tools_dir   := ../../tools
INCLUDES    := -I../../include

.PRECIOUS: %.b %.b.c

%.x: %.b.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.b.c: %.b
	$(tools_dir)/bintoc -f $< -o $@ -p user_bare

%.b: %.o
	$(LD) -o $@ $(LDFLAGS) -T ../user.lds $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

.PHONY: all clean

all: loop.x overflow.x put_a.x put_b.x

clean:
	rm -rf *.o *~ *.x *.b *.b.c

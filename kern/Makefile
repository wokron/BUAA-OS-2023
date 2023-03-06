include ./include.mk

INCLUDES    := -I../include/

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

.PHONY: all clean

all: $(targets)

clean:
	rm -rf *~ *.o

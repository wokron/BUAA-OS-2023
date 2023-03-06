INCLUDES    := -I../include/

targets     := elfloader.o print.o string.o

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

.PHONY: all clean

all: $(targets)

clean:
	rm -rf *~ *.o

include include.mk

.PHONY: all clean

all: $(targets)

%: %.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(targets)

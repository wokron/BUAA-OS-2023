user_dir    := ../user
tools_dir   := ../tools
INCLUDES    := -I../include -I$(user_dir)/include

include $(user_dir)/include.mk
USERLIB     := $(addprefix $(user_dir)/, $(USERLIB))
USERAPPS    := $(addprefix $(user_dir)/, $(USERAPPS))

FSLIB       := fs.o ide.o
FSIMGFILES  := rootfs/motd rootfs/newmotd $(USERAPPS) $(fs-files)

.PRECIOUS: %.b %.b.c
%.x: %.b.c
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $<

%.b.c: %.b
	$(tools_dir)/bintoc -f $< -o $@ -p fs

%.b: %.o $(USERLIB) $(FSLIB)
	$(LD) -o $@ $(LDFLAGS) -T $(user_dir)/user.lds $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: $(user_dir)/lib.h

.PHONY: all clean image

all: serv.x check.x $(FSLIB)

clean:
	rm -rf *~ *.o *.b.c *.b *.x

image: $(tools_dir)/fsformat
	dd if=/dev/zero of=../target/fs.img bs=4096 count=1024 2>/dev/null
	# using awk to remove paths with identical basename from FSIMGFILES
	$(tools_dir)/fsformat ../target/fs.img \
		$$(printf '%s\n' $(FSIMGFILES) | awk -F/ '{ ns[$$NF]=$$0 } END { for (n in ns) { print ns[n] } }')

targets  := rootfs symlink_check.x

.PHONY: rootfs

include ../include.mk

rootfs:
	rm -rf rootfs
	mkdir rootfs rootfs/lib rootfs/my
	echo 'Hello, world!' > rootfs/real
	ln -s /my/b rootfs/lib/a
	ln -s /real rootfs/my/b

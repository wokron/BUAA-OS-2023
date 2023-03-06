include include.mk

lab                     ?= $(shell cat .mos-this-lab 2>/dev/null || echo 6)

target_dir              := target
mos_elf                 := $(target_dir)/mos
user_disk               := $(target_dir)/fs.img
link_script             := kernel.lds

modules                 := lib init kern
targets                 := $(mos_elf)
syms_file               := $(target_dir)/prog.syms
gxemul_files            += $(mos_elf)

lab-ge = $(shell [ "$$(echo $(lab)_ | cut -f1 -d_)" -ge $(1) ] && echo true)

ifeq ($(call lab-ge,3),true)
	user_modules    += user/bare
endif

ifeq ($(call lab-ge,4),true)
	user_modules    += user
endif

ifeq ($(call lab-ge,5),true)
	user_modules    += fs
	targets         += fs-image
endif

gxemul_flags            += -T -C R3000 -M 64
CFLAGS                  += -DLAB=$(shell echo $(lab) | cut -f1 -d_)

objects                 := $(addsuffix /*.o, $(modules)) $(addsuffix /*.x, $(user_modules))
modules                 += $(user_modules)

.PHONY: all test tools $(modules) clean run dbg objdump fs-image clean-and-all

.ONESHELL:
clean-and-all: clean
	$(MAKE) all

test: export test_dir = tests/lab$(lab)
test: clean-and-all

include mk/tests.mk mk/profiles.mk
export CC CFLAGS LD LDFLAGS lab

all: $(targets)

$(target_dir):
	mkdir -p $@

tools:
	CC="$(HOST_CC)" CFLAGS="$(HOST_CFLAGS)" $(MAKE) --directory=$@

$(modules): tools
	$(MAKE) --directory=$@

$(mos_elf): $(modules) $(target_dir)
	$(LD) $(LDFLAGS) -o $(mos_elf) -N -T $(link_script) $(objects)

fs-image: $(target_dir) user
	$(MAKE) --directory=fs image fs-files="$(addprefix ../, $(fs-files))"

fs: user
user: lib

clean:
	for d in * tools/readelf user/* tests/*; do
		if [ -f $$d/Makefile ]; then
			$(MAKE) --directory=$$d clean
		fi
	done
	rm -rf *.o *~ $(target_dir) include/generated
	find . -name '*.objdump' -exec rm {} ';'

ifneq ($(prog),)
dbg:
	$(CROSS_COMPILE)nm -S '$(prog)' > $(syms_file)
	@gxemul_files=$(syms_file) gxemul_flags=-V $(MAKE) run
else
dbg: gxemul_flags += -V
dbg: run
endif

run: gxemul_flags += -E $(shell gxemul -H | grep -q oldtestmips && echo old)testmips \
			$(shell [ -f '$(user_disk)' ] && echo '-d $(user_disk)')
run:
	gxemul $(gxemul_flags) $(gxemul_files)

objdump:
	@find * \( -name '*.b' -o -path $(mos_elf) \) -exec sh -c \
	'$(CROSS_COMPILE)objdump {} -aldS > {}.objdump && echo {}.objdump' ';'

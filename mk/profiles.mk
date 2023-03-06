RELEASE_CFLAGS   := $(CFLAGS) -O2
RELEASE_LDFLAGS  := $(LDFLAGS) -O --gc-sections
DEBUG_CFLAGS     := $(CFLAGS) -O0 -g -ggdb -DMOS_DEBUG
DEBUG_LDFLAGS    := $(LDFLAGS)

ifeq ($(MOS_PROFILE),release)
	CFLAGS   := $(RELEASE_CFLAGS)
	LDFLAGS  := $(RELEASE_LDFLAGS)
else
	CFLAGS   := $(DEBUG_CFLAGS)
endif

.PHONY: debug release

debug: CFLAGS    := $(DEBUG_CFLAGS)
debug: LDFLAGS   := $(DEBUG_LDFLAGS)
debug: all

release: CFLAGS  := $(RELEASE_CFLAGS)
release: LDFLAGS := $(RELEASE_LDFLAGS)
release: all

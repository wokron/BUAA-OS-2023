lab-ge = $(shell [ "$$(echo $(lab)_ | cut -f1 -d_)" -ge $(1) ] && echo true)

ifeq ($(call lab-ge,3), true)
	targets  += bintoc
endif

ifeq ($(call lab-ge,5), true)
	targets  += fsformat
endif

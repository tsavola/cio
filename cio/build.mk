NAME		:= cio
VERSION		:= 0
SOURCES		:= $(wildcard cio/*.c)
CPPFLAGS	+= -pthread
LDFLAGS		+= -pthread

include build/library.mk

HEADERS		:= $(filter-out $(wildcard cio/*-internal.h),$(wildcard cio/*.h))

install::
	$(QUIET) install -d $(DEST_PREFIX)/include/cio
	$(QUIET) install $(HEADERS) $(DEST_PREFIX)/include/cio/

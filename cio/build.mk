MAP		?= array

NAME		:= cio
VERSION		:= 0
SOURCES_IGNORE	:= $(filter-out cio/map-$(MAP).c,$(wildcard cio/map-*.c))
SOURCES		:= $(filter-out $(SOURCES_IGNORE),$(wildcard cio/*.c))
CPPFLAGS	+= -pthread -DCIO_MAP_$(shell echo $(MAP) | tr a-z A-Z)
LDFLAGS		+= -pthread

include build/library.mk

HEADERS		:= $(filter-out $(wildcard cio/*-internal.h),$(wildcard cio/*.h))

install::
	$(QUIET) install -d $(DEST_PREFIX)/include/cio
	$(QUIET) install $(HEADERS) $(DEST_PREFIX)/include/cio/

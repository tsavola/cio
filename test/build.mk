NAME		:= test
SOURCES		:= $(wildcard test/*.c) $(wildcard test/*.cpp)
CPPFLAGS	:= -I.
LDFLAGS		:= -pthread
LIBS		:= $(O)/lib/libcio.a
DEPENDS		:= $(LIBS)

include build/test.mk

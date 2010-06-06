NAME		:= test
SOURCES		:= $(wildcard test/*.c) $(wildcard test/*.cpp)
CPPFLAGS	:= -I. -I$(O)
LDFLAGS		:= -pthread
LIBS		:= $(O)/lib/libcio.a
DEPENDS		:= $(LIBS)

include build/test.mk

TEST_SUITE	:= $(O)/test/suite.h
TEST_SOURCES	:= test/test_*.c test/test_*.cpp

$(O)/obj/test/main.o $(O)/obj/test/main.os: $(TEST_SUITE)

$(TEST_SUITE): $(TEST_SOURCES)
	$(call echo,Generate,$@)
	$(QUIET) mkdir -p $(dir $@)
	$(QUIET) ls -1 $(TEST_SOURCES) | sort | sed -r "s,test/test_([^.]+).*,test(\1);,g" > $@

CFLAGS		+= -g -Wall
CCFLAGS		+= -std=gnu99
CXXFLAGS	+= -std=c++0x

LIBRARIES	:= cio
BINARIES	:= httpd
TESTS		:= test

build: $(LIBRARIES) $(BINARIES)
cio: cio-static cio-shared
httpd: cio-shared
test: cio-static

include build/project.mk

CTAGS		:= ctags-exuberant
ETAGS		:= $(CTAGS) -e

SRCDIRS		:= $(LIBRARIES) $(BINARIES) $(TESTS)
SOURCES		:= $(shell find $(SRCDIRS) -name '*.[chS]' -o -name '*.[ch]pp')

ctags:: tags
etags:: TAGS

tags: $(SOURCES)
	$(call echo,Update,$@)
	$(QUIET) $(CTAGS) -f $@ $(SOURCES)

TAGS: $(SOURCES)
	$(call echo,Update,$@)
	$(QUIET) $(ETAGS) -f $@ $(SOURCES)

todo::
	$(QUIET) grep -n -i todo $(SOURCES) | sed -r -e "s,[[:space:]]*//,," \
		-e "s,[[:space:]]*/?\*+,," -e "s,[[:space:]]*\*+/[[:space:]]*,,"

doc:: doc/html/index.html

doc/html/index.html: $(SOURCES)
	$(call echo,Doxygen,doc/html)
	$(QUIET) doxygen

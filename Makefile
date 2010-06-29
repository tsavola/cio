CFLAGS		+= -g -Wall
CCFLAGS		+= -std=gnu99
CXXFLAGS	+= -std=c++0x

LIBRARIES	:= cio
BINARIES	:= httpd
OTHERS		:= python
TESTS		:= test python/test

build: $(LIBRARIES)
cio: cio-static cio-shared
test: cio-static
python: cio-shared
python/test: python
httpd: cio-shared

include build/project.mk

CTAGS		:= ctags-exuberant
ETAGS		:= $(CTAGS) -e

SRCDIRS		:= $(LIBRARIES) $(BINARIES) $(OTHERS) $(TESTS)
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

DOXYGEN		:= doxygen
DOXYGEN_VERSION	:= Doxygen $(shell $(DOXYGEN) --version)

doc:: doc/html/index.html

doc/html/index.html: $(SOURCES)
	$(call echo,Doxygen,doc/html)
	$(QUIET) doxygen

GIT_COMMIT	= $(shell git rev-parse HEAD)
GIT_BRANCH	= $(shell git rev-parse --symbolic-full-name --abbrev-ref HEAD)
GIT_REMOTE	= $(shell git config branch.$(GIT_BRANCH).remote)
GIT_URL		= $(shell git config remote.$(GIT_REMOTE).url)

doc-github:: doc
	$(QUIET) ( \
		set -x && \
		test -f Makefile && \
		test -d cio && \
		test -d .git && \
		cd doc/html && \
		rm -rf .git && \
		git init && \
		git add . && \
		git commit -m $(GIT_COMMIT) && \
		git push -f $(GIT_URL) $(GIT_BRANCH):gh-pages && \
		rm -rf .git \
	)

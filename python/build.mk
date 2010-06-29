include python/common.mk

PYTHONNAME	?= $(notdir $(shell readlink -f $(shell which $(PYTHON))))
PYTHONDIR	?= $(PREFIX)/lib/$(PYTHONNAME)/site-packages

O_NAME		:= $(O)/lib/python/_cio.so
DEST_PYTHONDIR	:= $(DESTDIR)$(PYTHONDIR)

build::
	$(QUIET) cd python && O=../$(O) $(PYTHON) setup.py build_ext -b ../$(O)/lib/python -t ../$(O)/obj/python

install::
	$(QUIET) cd python && $(PYTHON) setup.py install_lib --skip-build -d $(DEST_PYTHONDIR) -O1
	$(QUIET) cd python && $(PYTHON) setup.py install_lib --skip-build -d $(DEST_PYTHONDIR) -b ../$(O)/lib/python

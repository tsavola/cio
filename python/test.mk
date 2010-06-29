NAME		:= python/test.py

include python/common.mk

build::

check::
	$(call echo,Check,$(NAME))
	$(QUIET) LD_LIBRARY_PATH=$(O)/lib PYTHONPATH=python:$(O)/lib/python $(PYTHON) $(NAME)

#
# Copyright (c) 2010  Timo Savola
#

__all__ = [
	"launch",

	"read",
	"write",

	"nonblock",
]

import fcntl
import os

import _cio

def launch(routine, *args):
	if args:
		callable = lambda: routine(*args)
	else:
		callable = routine

	_cio.launch(callable)

def read(fd, buf, size=None):
	if size is None:
		size = len(buf)

	return _cio.read(fd, buf, size)

def write(fd, buf, size=None):
	if size is None:
		size = len(buf)

	return _cio.write(fd, buf, size)

def nonblock(fd):
	flags = fcntl.fcntl(fd, fcntl.F_GETFL)
	fcntl.fcntl(fd, fcntl.F_SETFL, flags | os.O_NONBLOCK)

	return fd

#
# Copyright (c) 2010  Timo Savola
#

__all__ = [
	"routine",

	"channel",

	"close",

	"read",
	"write",

	"connect",
	"accept",
	"recv",
	"send",

	"nonblock",
]

import fcntl
import os

import _cio

def routine(routine, *args):
	if args:
		callable = lambda: routine(*args)
	else:
		callable = routine

	_cio.routine(callable)

class channel(object):
	__slots__ = ["_channel"]

	def __init__(self):
		self._channel = _cio.channel()

	def __iter__(self):
		while True:
			item, ok = self._channel.read()
			if ok:
				yield item
			else:
				break

	def close(self):
		self._channel.close()

	def read(self):
		return self._channel.read()

	def write(self, item=None):
		return self._channel.write(item)

def close(fd):
	return _cio.close(fd)

def read(fd, buf, size=None):
	if size is None:
		size = len(buf)

	return _cio.read(fd, buf, size)

def write(fd, buf, size=None):
	if size is None:
		size = len(buf)

	return _cio.write(fd, buf, size)

def connect(sockfd, addr):
	if isinstance(addr, (tuple, list)):
		host, port = addr
		if not host:
			host = None
		addr = host, str(port)

	_cio.connect(sockfd, addr)

def accept(sockfd):
	return _cio.accept(sockfd)

def recv(sockfd, buf, size=None, flags=0):
	if size is None:
		size = len(buf)

	return _cio.recv(sockfd, buf, size, flags)

def send(sockfd, buf, size=None, flags=0):
	if size is None:
		size = len(buf)

	return _cio.send(sockfd, buf, size, flags)

def nonblock(fd):
	flags = fcntl.fcntl(fd, fcntl.F_GETFL)
	fcntl.fcntl(fd, fcntl.F_SETFL, flags | os.O_NONBLOCK)

	return fd

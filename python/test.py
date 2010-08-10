import os
import socket as socketlib
import unittest

import cio

class IO(unittest.TestCase):
	def test(self):
		count = 8
		pipe = os.pipe()

		cio.nonblock(pipe[0])
		cio.nonblock(pipe[1])

		for i in range(count):
			cio.routine(self.writer, os.dup(pipe[1]), i)

		self.reader(pipe[0], count)

		cio.close(pipe[0])
		cio.close(pipe[1])

	def writer(self, fd, i):
		buf = str(i)
		n = cio.write(fd, buf)
		print("write(%s) = %d" % (repr(buf), n))

		cio.close(fd)

	def reader(self, fd, total):
		count = 0
		while count < total:
			buf = bytearray(3)
			n = cio.read(fd, buf)
			print("read() -> %s" % repr(buf[:n]))

			count += n

class Socket(unittest.TestCase):
	def test(self):
		count = 8
		address = "", 1234

		listener = socketlib.socket(socketlib.AF_INET, socketlib.SOCK_STREAM)
		listener.bind(address)
		listener.listen(socketlib.SOMAXCONN)

		for i in range(count):
			cio.routine(self.client, address, i)

		self.server(listener, 8)

	def server(self, listener, count):
		for i in range(count):
			socketfd = cio.accept(listener.fileno())
			print("accept() -> %d" % socketfd)

			cio.routine(self.server_handle, socketfd)

		listener.close()

		for i in range(10000):
			cio.write(1, "")

	def server_handle(self, fd):
		while True:
			buf = bytearray(4)
			n = cio.recv(fd, buf)
			if n == 0:
				break
			print("recv(%d) -> %s" % (fd, repr(buf[:n])))

		cio.close(fd)

	def client(self, address, i):
		socket = socketlib.socket(socketlib.AF_INET, socketlib.SOCK_STREAM)
		cio.connect(socket.fileno(), address)
		print("connect()")

		buf = "hello %d" % i
		n = cio.send(socket.fileno(), buf)
		print("send(%s) = %d" % (repr(buf), n))

		socket.close()

class Channel(unittest.TestCase):
	def test(self):
		c = cio.channel()
		cio.routine(self.reader, c)
		cio.routine(self.writer, c)

		cio.nonblock(1)
		for i in range(10):
			cio.write(1, "%d\n" % i)

	def writer(self, c):
		print(c.write("1"))
		print(c.write("2"))
		print("writer done")

	def reader(self, c):
		print(c.read())
		print(c.read())
		print("reader done")

if __name__ == "__main__":
	unittest.main()

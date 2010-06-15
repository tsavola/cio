import os
import socket as socketlib

import cio

def test_io():
	count = 8
	pipe = os.pipe()

	cio.nonblock(pipe[0])
	cio.nonblock(pipe[1])

	for i in range(count):
		cio.launch(io_writer, os.dup(pipe[1]), i)

	io_reader(pipe[0], count)

	os.close(pipe[0])
	os.close(pipe[1])

def io_writer(fd, i):
	buf = str(i)
	n = cio.write(fd, buf)
	print("write(%s) = %d" % (repr(buf), n))

	os.close(fd)

def io_reader(fd, total):
	count = 0
	while count < total:
		buf = bytearray(3)
		n = cio.read(fd, buf)
		print("read() -> %s" % repr(buf[:n]))

		count += n

def test_socket():
	count = 8
	address = "", 1234

	listener = socketlib.socket(socketlib.AF_INET, socketlib.SOCK_STREAM)
	listener.bind(address)
	listener.listen(socketlib.SOMAXCONN)

	for i in range(count):
		cio.launch(socket_client, address, i)

	socket_server(listener, 8)

def socket_server(listener, count):
	for i in range(count):
		socketfd = cio.accept(listener.fileno())
		print("accept() -> %d" % socketfd)

		cio.launch(socket_server_handle, socketfd)

	listener.close()

	for i in range(10000):
		cio.write(1, "")

def socket_server_handle(fd):
	while True:
		buf = bytearray(4)
		n = cio.recv(fd, buf)
		if n == 0:
			break
		print("recv(%d) -> %s" % (fd, repr(buf[:n])))

	os.close(fd)

def socket_client(address, i):
	socket = socketlib.socket(socketlib.AF_INET, socketlib.SOCK_STREAM)
	cio.connect(socket.fileno(), address)
	print("connect()")

	buf = "hello %d" % i
	n = cio.send(socket.fileno(), buf)
	print("send(%s) = %d" % (repr(buf), n))

	socket.close()

if __name__ == "__main__":
	test_io()
	test_socket()

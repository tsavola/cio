import os

import cio

def main():
	count = 8
	pipe = os.pipe()

	cio.nonblock(pipe[0])
	cio.nonblock(pipe[1])

	for i in range(count):
		cio.launch(writer, os.dup(pipe[1]), i)

	reader(pipe[0], count)

	os.close(pipe[0])
	os.close(pipe[1])

def writer(fd, i):
	buf = str(i)
	n = cio.write(fd, buf)
	print("write(%s) = %d" % (repr(buf), n))

	os.close(fd)

def reader(fd, total):
	count = 0
	while count < total:
		buf = bytearray(3)
		n = cio.read(fd, buf)
		print("read() -> %s" % repr(buf[:n]))

		count += n

if __name__ == "__main__":
	main()

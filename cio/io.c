/*
 * Copyright (c) 2010  Timo Savola
 */

#include "io.h"

#include <errno.h>
#include <stdbool.h>

#include <sys/sendfile.h>
#include <unistd.h>

#include "sched.h"

enum cio_io_type {
	CIO_IO_READ,
	CIO_IO_WRITE,
	CIO_IO_SENDFILE,
};

/**
 * @param event     CIO_INPUT or CIO_OUTPUT
 * @param type      I/O call to make
 * @param fd        primary file descriptor
 * @param extra_fd  secondary file descriptor (unused by read and write)
 * @param buf       buffer to read from or write to (unused by sendfile)
 * @param offset    source file offset (must be NULL for read and write)
 * @param count     number of bytes to transfer
 * @param full      try to transfer only some bytes or as much as possible?
 */
static ssize_t cio_io(int event, enum cio_io_type type, int fd, int extra_fd, void *buf, off_t *offset, size_t count, bool full)
{
	struct cio_context context;
	ssize_t len;
	int err;
	ssize_t ret;

	if (cio_register(fd, event, &context) < 0)
		return -1;

	cio_yield(&context);

	for (len = 0; len < count; ) {
		switch (type) {
		case CIO_IO_READ:
			ret = read(fd, buf + len, count - len);
			break;

		case CIO_IO_WRITE:
			ret = write(fd, buf + len, count - len);
			break;

		case CIO_IO_SENDFILE:
			ret = sendfile(fd, extra_fd, offset, count - len);
			break;
		}

		if (ret < 0) {
			if ((errno == EAGAIN || errno == EINTR) && (full || len == 0)) {
				cio_yield(&context);
				continue;
			}

			if (len == 0) {
				len = -1;
				err = errno;
			}
			break;
		}

		if (ret == 0)
			break;

		if (offset)
			*offset += ret;

		len += ret;
	}

	cio_unregister(fd);

	if (len < 0)
		errno = err;

	return len;
}

/**
 * Read one or more bytes from a file descriptor.
 *
 * @param fd       file descriptor
 * @param buf      buffer
 * @param bufsize  maximum number of bytes to read
 *
 * @retval >0 the number of bytes read
 * @retval 0 on end-of-file
 * @retval -1 on error with @c errno set
 *
 * @pre @p fd should be in non-blocking mode
 *
 * @see @c read(2)
 */
ssize_t cio_read(int fd, void *buf, size_t bufsize)
{
	return cio_io(CIO_INPUT, CIO_IO_READ, fd, -1, buf, NULL, bufsize, false);
}

/**
 * Read all requested bytes from a file descriptor.
 *
 * @param fd       file descriptor
 * @param buf      buffer
 * @param bufsize  number of bytes to read
 *
 * @retval >0 the number of bytes read; may be less than @p bufsize due to
 *            end-of-file or error
 * @retval 0 on end-of-file before anything could be read
 * @retval -1 on error with @c errno set
 *
 * @pre @p fd should be in non-blocking mode
 *
 * @see @c read(2)
 */
ssize_t cio_read_full(int fd, void *buf, size_t bufsize)
{
	return cio_io(CIO_INPUT, CIO_IO_READ, fd, -1, buf, NULL, bufsize, true);
}

/**
 * Write all bytes to a file descriptor.
 *
 * @param fd       file descriptor
 * @param buf      buffer
 * @param bufsize  number of bytes to write
 *
 * @retval >0 the number of bytes written; may be less than @p bufsize due to
 *            end-of-file or error
 * @retval 0 on end-of-file before anything could be written
 * @retval -1 on error with @c errno set
 *
 * @pre @p fd should be in non-blocking mode
 *
 * @see @c write(2)
 */
ssize_t cio_write(int fd, const void *buf, size_t bufsize)
{
	return cio_io(CIO_OUTPUT, CIO_IO_WRITE, fd, -1, (void *) buf, NULL, bufsize, true);
}

/**
 * Transfer data between file descriptors.  If @p offset is not @c NULL, it
 * will be updated according to the number of bytes transferred.
 *
 * @param out_fd  target file descriptor
 * @param in_fd   source file descriptor
 * @param offset  pointer to file offset or @c NULL
 * @param count   number of bytes to transfer
 *
 * @retval >=0 the number of bytes transferred
 * @retval -1 on error with @c errno set
 *
 * @pre @p fd should be in non-blocking mode
 *
 * @note any restrictions imposed by the platform's @c sendfile()
 *       implementation apply also to this function
 *
 * @bug if @p in_fd returns @c EAGAIN (and there is no other I/O going on),
 *      this function busyloops
 *
 * @see @c sendfile(2)
 */
ssize_t cio_sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
	return cio_io(CIO_OUTPUT, CIO_IO_SENDFILE, out_fd, in_fd, NULL, offset, count, true);
}

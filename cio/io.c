/*
 * Copyright (c) 2010  Timo Savola
 */

#include "io-internal.h"
#include "io.h"
#include "socket.h"

#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <unistd.h>

#include "sched.h"
#include "trace.h"

/**
 * @param event     CIO_INPUT or CIO_OUTPUT
 * @param type      I/O call to make
 * @param fd        primary file descriptor
 * @param extra_fd  secondary file descriptor (used by sendfile)
 * @param buf       buffer to read from or write to (not used by sendfile)
 * @param offset    source file offset (used by sendfile)
 * @param count     number of bytes to transfer
 * @param flags     (used by recv/send)
 */
static ssize_t cio_io(int event, enum cio_io_type type, int fd, int extra_fd, void *buf, off_t *offset, size_t count, int flags)
{
	struct cio_context context;
	ssize_t len = -1;
	ssize_t ret;

	cio_tracef("%s: alloc context %p", __func__, &context);

	if (cio_register(fd, event, &context) < 0)
		goto no_register;

	if (cio_yield(&context) < 0)
		goto no_yield;

	for (len = 0; len < count; ) {
		switch (type) {
		case CIO_IO_READ:
			ret = read(fd, buf + len, count - len);
			break;

		case CIO_IO_WRITE:
			ret = write(fd, buf + len, count - len);
			break;

		case CIO_IO_RECV:
			ret = recv(fd, buf + len, count - len, flags & ~MSG_WAITALL);
			break;

		case CIO_IO_SEND:
			ret = send(fd, buf + len, count - len, flags & ~MSG_WAITALL);
			break;

		case CIO_IO_SENDFILE:
			ret = sendfile(fd, extra_fd, offset, count - len);
			break;
		}

		if (ret < 0) {
			if (errno == EAGAIN && ((flags & MSG_WAITALL) || len == 0)) {
				if (cio_yield(&context) >= 0)
					continue;
			}

			if (len == 0)
				len = -1;

			break;
		}

		if (ret == 0)
			break;

		if (offset)
			*offset += ret;

		len += ret;
	}

no_yield:
	cio_unregister(fd);

no_register:
	cio_tracef("%s: free context %p", __func__, &context);

	return len;
}

/**
 * Read from a file descriptor.
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
	return cio_io(CIO_INPUT, CIO_IO_READ, fd, -1, buf, NULL, bufsize, 0);
}

/**
 * Write to a file descriptor.
 *
 * @param fd       file descriptor
 * @param buf      buffer
 * @param bufsize  maximum number of bytes to write
 *
 * @retval >0 the number of bytes written
 * @retval 0 on end-of-file
 * @retval -1 on error with @c errno set
 *
 * @pre @p fd should be in non-blocking mode
 *
 * @see @c write(2)
 */
ssize_t cio_write(int fd, const void *buf, size_t bufsize)
{
	return cio_io(CIO_OUTPUT, CIO_IO_WRITE, fd, -1, (void *) buf, NULL, bufsize, 0);
}

/**
 * TODO
 */
ssize_t cio_recv(int sockfd, void *buf, size_t len, int flags)
{
	return cio_io(CIO_INPUT, CIO_IO_READ, sockfd, -1, buf, NULL, len, flags);
}

/**
 * TODO
 */
ssize_t cio_send(int sockfd, const void *buf, size_t len, int flags)
{
	return cio_io(CIO_OUTPUT, CIO_IO_WRITE, sockfd, -1, (void *) buf, NULL, len, flags);
}

/**
 * Transfer data between file descriptors.  If @p offset is not @c NULL, it
 * will be updated according to the number of bytes transferred.
 *
 * @param out_fd  target file descriptor
 * @param in_fd   source file descriptor
 * @param offset  pointer to file offset or @c NULL
 * @param count   maximum number of bytes to transfer
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
	return cio_io(CIO_OUTPUT, CIO_IO_SENDFILE, out_fd, in_fd, NULL, offset, count, 0);
}

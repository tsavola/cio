/*
 * Copyright (c) 2010  Timo Savola
 */

#define _GNU_SOURCE

#include "socket.h"

#include <errno.h>
#include <stdbool.h>

#include "sched.h"

/**
 * Initiate a connection on a socket.
 *
 * @param sockfd   socket file descriptor
 * @param addr     socket address
 * @param addrlen  size of the address
 *
 * @retval 0 on success
 * @retval -1 on error with @c errno set
 *
 * @pre @p fd should be in non-blocking mode
 *
 * @see @c connect(2)
 */
int cio_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int ret;

	do {
		ret = connect(sockfd, addr, addrlen);
	} while (ret < 0 && errno == EINTR);

	if (ret < 0 && errno == EINPROGRESS) {
		struct cio_context context;
		int err;

		if (cio_register(sockfd, CIO_OUTPUT, &context) < 0)
			return -1;

		while (true) {
			cio_yield(&context);

			socklen_t len = sizeof (ret);
			if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &ret, &len) < 0) {
				err = errno;
				ret = -1;
				break;
			}

			if (ret < 0) {
				if (errno == EALREADY || errno == EINTR)
					continue;

				err = errno;
			}

			break;
		}

		cio_unregister(sockfd);

		if (ret < 0)
			errno = err;
	}

	return ret;
}

/**
 * Accept a connection on a socket.
 *
 * @param sockfd   socket file descriptor
 * @param addr     buffer for socket address or @c NULL
 * @param addrlen  size of the address buffer or @c NULL
 *
 * @retval >=0 a new connection socket file descriptor
 * @retval -1 on error with @c errno set
 *
 * @pre @p fd should be in non-blocking mode
 *
 * @see @c accept(2)
 */
int cio_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	return cio_accept4(sockfd, addr, addrlen, 0);
}

/**
 * Accept a connection on a socket.
 *
 * @param flags  for the new file descriptor
 *
 * @see cio_accept()
 * @see @c accept4(2)
 */
int cio_accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags)
{
	struct cio_context context;
	int fd;
	int err;

	if (cio_register(sockfd, CIO_INPUT, &context) < 0)
		return -1;

	while (true) {
		cio_yield(&context);

		fd = accept4(sockfd, addr, addrlen, flags);

		if (fd < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;

			err = errno;
		}

		break;
	}

	cio_unregister(sockfd);

	if (fd < 0)
		errno = err;

	return fd;
}

/* cio_recv() and cio_send() are implemented in io.c */

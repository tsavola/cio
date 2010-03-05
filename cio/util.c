/*
 * Copyright (c) 2010  Timo Savola
 */

#include "util.h"

#include <fcntl.h>
#include <unistd.h>

/**
 * Set a file descriptor to non-blocking mode.
 *
 * @param fd  a file descriptor
 *
 * @retval fd on success
 * @retval -1 on error with @c errno set
 */
int cio_nonblock(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL);
	if (flags < 0)
		return -1;

	if ((flags & O_NONBLOCK) == 0 && fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
		return -1;

	return fd;
}

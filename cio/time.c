/*
 * Copyright (c) 2010  Timo Savola
 */

#include "time.h"

#include <errno.h>
#include <stdint.h>

#include <unistd.h>

#include <sys/timerfd.h>

#include "sched.h"

/**
 * @bug Broken
 */
int cio_sleep(const struct timespec *interval)
{
	int fd;
	struct cio_context context;
	int ret;
	int err;
	uint64_t count;

	fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (fd < 0)
		return -1;

	struct itimerspec spec = {
		.it_interval = *interval,
	};

	ret = timerfd_settime(fd, 0, &spec, NULL);
	if (ret < 0)
		goto fail;

	ret = cio_register(fd, CIO_INPUT, &context);
	if (ret < 0)
		goto fail;

	do {
		cio_yield(&context);

		ret = read(fd, &count, sizeof (count));

		if (ret < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;

			err = errno;
			break;
		}
	} while (count == 0);

	cio_unregister(fd);

	if (ret < 0)
		errno = err;

fail:
	close(fd);

	return ret;
}

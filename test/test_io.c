#include <stdbool.h>
#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>

#include <cio/routine.h>
#include <cio/io.h>
#include <cio/util.h>

#include "assert.h"

#define ROUTINE_COUNT  1000
#define WRITE_COUNT    1000

struct reader_args {
	int io_fd;
	int exit_fd;
};

static void reader(void *data)
{
	struct reader_args *args = data;
	int fd;
	int i;

	fd = dup(args->io_fd);
	assert(fd >= 0);

	while (true) {
		printf("reading\n");

		ssize_t ret = cio_read(fd, &i, sizeof (i));
		assert(ret >= 0);

		if (ret == 0)
			break;

		assert(ret == sizeof (i));

		printf("read %d\n", i);
	}

	close(fd);

	printf("reader exiting\n");

	char buf = 0;
	assert(cio_write(dup(args->exit_fd), &buf, 1) == 1);
}

static void writer(int fd)
{
	for (int i = 0; i < WRITE_COUNT; i++) {
		printf("writing %d\n", i);

		assert(cio_write(fd, &i, sizeof (i)) == sizeof (i));

		printf("wrote %d\n", i);
	}

	printf("writer exiting\n");
}

void test_io(void)
{
	int io_fd[2];
	int exit_fd[2];

	assert(pipe(io_fd) == 0);
	assert(cio_nonblock(io_fd[0]) >= 0);
	assert(cio_nonblock(io_fd[1]) >= 0);

	assert(pipe(exit_fd) == 0);
	assert(cio_nonblock(exit_fd[0]) >= 0);
	assert(cio_nonblock(exit_fd[1]) >= 0);

	for (int i = 0; i < ROUTINE_COUNT; i++) {
		struct reader_args args = {
			.io_fd = io_fd[0],
			.exit_fd = exit_fd[1],
		};

		assert(cio_launch(reader, &args, sizeof (args)) == 0);
	}

	writer(io_fd[1]);

	close(io_fd[1]);

	for (int i = 0; i < ROUTINE_COUNT; i++) {
		char buf;
		assert(cio_read(exit_fd[0], &buf, 1) == 1);
	}

	close(io_fd[0]);
	close(exit_fd[0]);
	close(exit_fd[1]);
}

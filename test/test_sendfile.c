#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <cio/io.h>
#include <cio/routine.h>
#include <cio/sched.h>
#include <cio/util.h>

#include "assert.h"

struct args {
	int fd;
	size_t size;
};

static void receiver(void *data)
{
	struct args *args = data;
	char buf;
	size_t len = 0;
	ssize_t ret;

	printf("receiver\n");

	while (true) {
		ret = cio_read(args->fd, &buf, 1);
		assert(ret >= 0);

		if (ret == 0)
			break;

		len += ret;
	}

	assert(len == args->size);

	close(args->fd);

	printf("receiver done\n");

	exit(0);
}

void test_sendfile(void)
{
	int sock_fd[2];
	int file_fd;
	struct stat file_stat;

	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sock_fd) == 0);
	assert(cio_nonblock(sock_fd[0]) >= 0);
	assert(cio_nonblock(sock_fd[1]) >= 0);

	file_fd = open("/bin/grep", O_RDONLY);
	assert(file_fd >= 0);
	assert(cio_nonblock(file_fd) >= 0);

	assert(fstat(file_fd, &file_stat) == 0);

	struct args args = {
		.fd = sock_fd[1],
		.size = file_stat.st_size,
	};

	cio_launch(receiver, &args, sizeof (args));

	printf("sendfile\n");

	assert(cio_sendfile(sock_fd[0], file_fd, NULL, file_stat.st_size) == file_stat.st_size);

	printf("sendfile done\n");

	close(file_fd);
	close(sock_fd[0]);

	struct cio_context dirty_hack;
	cio_yield(&dirty_hack);
}

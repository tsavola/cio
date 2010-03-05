#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>

#include <cio/channel.h>
#include <cio/io.h>
#include <cio/routine.h>
#include <cio/util.h>

#include "assert.h"

static bool routine2_done;

static void routine0(void)
{
	FILE *file;
	int fd;

	printf("routine 0\n");

	file = popen("while true; do sleep 0.2; echo; done", "r");
	assert(file);

	fd = fileno(file);
	assert(fd >= 0);

	assert(cio_nonblock(fd) >= 0);

	while (!routine2_done) {
		printf("ticker read\n");

		unsigned char buf;
		assert(cio_read(fd, &buf, 1) == 1);

		printf("ticker read done\n");
	}

	/* don't pclose file */

	printf("routine 0 done\n");
}

static void routine1(void *arg)
{
	struct cio_channel *c = *(void **) arg;
	int fd;

	printf("routine 1\n");

	fd = open("/dev/random", O_RDONLY);
	assert(fd >= 0);

	assert(cio_nonblock(fd) >= 0);

	for (int i = 0; i < 4; i++) {
		printf("file read\n");

		unsigned char buf;
		assert(cio_read(fd, &buf, 1) == 1);

		printf("file read done\n");
		printf("channel write\n");

		assert(cio_channel_write(c, &buf, 1) == 1);

		printf("channel write done\n");
	}

	close(fd);

	cio_channel_unref(c);

	printf("routine 1 done\n");
}

static void routine2(void *arg)
{
	struct cio_channel *c = *(void **) arg;

	printf("routine 2\n");

	for (int i = 0; i < 4; i++) {
		printf("channel read\n");

		unsigned char buf;
		assert(cio_channel_read(c, &buf, 1) == 1);

		printf("channel read done\n");

		printf("0x%02x\n", (unsigned int) buf);
	}

	cio_channel_unref(c);

	printf("routine 2 done\n");

	routine2_done = true;
}

void test_channel(void)
{
	struct cio_channel *c;

	printf("test 2\n");

	c = cio_channel_create(1);
	assert(c);

	assert(cio_channel_ref(c) == c);

	cio_launch(routine1, &c, sizeof (c));
	cio_launch(routine2, &c, sizeof (c));

	routine0();

	printf("test 2 done\n");
}

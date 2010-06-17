#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include <cio/channel.h>
#include <cio/io.h>
#include <cio/routine.h>
#include <cio/util.h>

#include "assert.h"

static void routine(void *arg)
{
	struct cio_channel *chan = *(void **) arg;
	FILE *file;
	int fd;

	file = popen("while true; do sleep 0.1; echo; done", "r");
	assert(file);

	fd = fileno(file);
	assert(fd >= 0);

	assert(cio_nonblock(fd) >= 0);

	while (true) {
		unsigned char buf;
		assert(cio_read(fd, &buf, 1) == 1);

		assert(cio_channel_write(chan, &fd, sizeof (fd)) == 1);
	}

	/* don't pclose file */

	cio_channel_unref(chan);
}

void test_channel_select(void)
{
	const int count = 8;
	struct cio_channel *channels[count];

	for (int i = 0; i < count; i++) {
		struct cio_channel *c = cio_channel_create(sizeof (int));
		assert(c);

		assert(cio_channel_ref(c) == c);
		cio_launch(routine, &c, sizeof (c));

		channels[i] = c;
	}

	for (int i = 0; i < count * 4; i++) {
		struct cio_channel_op ops[count];
		int buf[count];

		for (int i = 0; i < count; i++) {
			struct cio_channel_op *op = &ops[i];

			op->type = CIO_CHANNEL_READ;
			op->channel = channels[i];
			op->item = &buf[i];
			op->item_size = sizeof (int);
		}

		int selected;
		int ret = cio_channel_select(ops, count, &selected);
		assert(ret == 1);
		assert(selected >= 0);
		assert(selected < count);

		printf("#%d -> %d\n", selected, buf[selected]);
	}
}

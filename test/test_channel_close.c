#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include <cio/channel.h>
#include <cio/io.h>
#include <cio/routine.h>
#include <cio/trace.h>
#include <cio/util.h>

#include "assert.h"

static int write_count_item;
static int write_count_closed;

static int read_count_item;
static int read_count_closed;

struct args {
	int id;
	struct cio_channel *exited;
	struct cio_channel *counter;
};

static void launch(void (*routine)(void *), int id, struct cio_channel *exited, struct cio_channel *counter, int trace_id)
{
	struct args args = {
		.id      = id,
		.exited  = cio_channel_ref(exited),
		.counter = cio_channel_ref(counter),
	};

	cio_tracef("%-18s [%4d] cio_launch(%d)", __func__, trace_id, id);
	assert(cio_launch(routine, &args, sizeof (struct args)) == 0);
}

static void cleanup(struct args *args)
{
	cio_channel_unref(args->counter);

	cio_tracef("%-18s [%4d] cio_channel_write(exited)", __func__, args->id);
	assert(cio_channel_write(args->exited, NULL, 0) == 1);

	cio_channel_unref(args->exited);
}

static void closer(void *void_args)
{
	struct args *args = void_args;

	cio_tracef("%-18s [%4d] cio_channel_close(counter)", __func__, args->id);
	cio_channel_close(args->counter);

	cleanup(args);
}

static void writer(void *void_args)
{
	struct args *args = void_args;

	for (int i = 0; i < 400; i++) {
		int item = 12345678;

		cio_tracef("%-18s [%4d] cio_channel_write(counter)", __func__, args->id);
		int ret = cio_channel_write(args->counter, &item, sizeof (item));
		assert(ret >= 0 && ret <= 1);

		if (ret == 0) {
			write_count_closed++;
			break;
		}

		write_count_item++;

		if (i == 200)
			launch(closer, 1000 + args->id, args->exited, args->counter, args->id);
	}

	cleanup(args);
}

static void reader(void *void_args)
{
	struct args *args = void_args;

	while (true) {
		int item;

		cio_tracef("%-18s [%4d] cio_channel_read(counter)", __func__, args->id);
		int ret = cio_channel_read(args->counter, &item, sizeof (item));
		assert(ret >= 0 && ret <= 1);

		if (ret == 0) {
			read_count_closed++;
			break;
		}

		assert(item == 12345678);
		read_count_item++;
	}

	cleanup(args);
}

static void counter_op(struct cio_channel_op *op, struct cio_channel *counter, int *item, bool write)
{
	if (write)
		op->type = CIO_CHANNEL_WRITE;
	else
		op->type = CIO_CHANNEL_READ;

	op->channel = counter;
	op->item = item;
	op->item_size = sizeof (int);
}

void test_channel_close(void)
{
	int id = 0;
	struct cio_channel *exited = cio_channel_create(0);
	struct cio_channel *counter = cio_channel_create(sizeof (int));

	for (int i = 0; i < 10; i++)
		launch(writer, 1 + i, exited, counter, id);

	for (int i = 0; i < 40; i++)
		launch(reader, 101 + i, exited, counter, id);

	bool closed = false;
	int turn = 0;

	for (int exited_count = 0; exited_count < 10 * 2 + 40; ) {
		int item = 12345678;
		struct cio_channel_op ops[2];
		int nops = 0;

		if (!closed && (turn & 1)) {
			counter_op(ops + nops, counter, &item, (turn & 2));
			nops++;
		}

		ops[nops].type = CIO_CHANNEL_READ;
		ops[nops].channel = exited;
		ops[nops].item = NULL;
		ops[nops].item_size = 0;
		nops++;

		if (!closed && !(turn & 1)) {
			counter_op(ops + nops, counter, &item, !(turn & 2));
			nops++;
		}

		cio_tracef("%-18s [%4d] cio_channel_select(%d)", __func__, id, nops);
		int index;
		int ret = cio_channel_select(ops, nops, &index);
		assert(ret >= 0);

		if (ops[index].channel == exited) {
			assert(ret == 1);

			exited_count++;
		}

		if (ops[index].channel == counter) {
			if (ret == 1) {
				switch (ops[index].type) {
				case CIO_CHANNEL_READ:
					assert(*(int *) ops[index].item == 12345678);
					read_count_item++;
					break;

				case CIO_CHANNEL_WRITE:
					write_count_item++;
					break;
				}
			}

			if (ret == 0)
				closed = true;
		}

		turn = (turn + 1) & 3;
	}

	cio_channel_unref(counter);
	cio_channel_unref(exited);

	printf("write item   = %d\n", write_count_item);
	printf("read  item   = %d\n", read_count_item);

	printf("write closed = %d\n", write_count_closed);
	printf("read  closed = %d\n", read_count_closed);

	assert(write_count_item == read_count_item);

	assert(write_count_closed == 10);
	assert(read_count_closed == 40);
}

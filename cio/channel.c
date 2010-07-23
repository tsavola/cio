/*
 * Copyright (c) 2010  Timo Savola
 */

#include "channel.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "atomic-internal.h"
#include "list-internal.h"
#include "sched.h"
#include "trace.h"

struct cio_channel {
	int refs;
	size_t item_size;
	bool closed;
	struct cio_list read_list;
	struct cio_list write_list;
};

struct cio_channel_wait {
	int id;
	void *item;
	struct cio_context *context;
	bool closed;
	struct cio_channel_wait *next;
};

static int cio_channel_check(struct cio_channel *c, size_t item_size)
{
	if (item_size != c->item_size) {
		errno = EINVAL;
		return -1;
	}

	return 0;
}

static void cio_channel_wait_append(struct cio_list *list, struct cio_channel_wait *node, int id, void *item, struct cio_context *context)
{
	node->id = id;
	node->item = item;
	node->context = context;
	node->closed = false;

	cio_list_append(struct cio_channel_wait, list, node);
}

static struct cio_channel_wait *cio_channel_wait_head(struct cio_list *list)
{
	return cio_list_head(struct cio_channel_wait, list);
}

static void cio_channel_wait_remove_head(struct cio_list *list)
{
	cio_list_remove_head(struct cio_channel_wait, list);
}

static int cio_channel_wait(struct cio_list *list, void *item)
{
	struct cio_context context;
	struct cio_channel_wait node;
	int ret;

	cio_tracef("%s: alloc context %p", __func__, &context);

	cio_channel_wait_append(list, &node, 1, item, &context);
	ret = cio_yield(&context);
	cio_channel_wait_remove_head(list);

	cio_tracef("%s: free context %p", __func__, &context);

	if (ret < 0)
		return -1;

	if (node.closed)
		return 0;

	return 1;
}

static struct cio_list *cio_channel_wait_list(const struct cio_channel_op *op)
{
	if (op->type == CIO_CHANNEL_READ)
		return &op->channel->read_list;
	else
		return &op->channel->write_list;
}

/**
 * Create a new channel with a reference count of 1.
 *
 * @param item_size  size of items passed through this channel (may be 0)
 *
 * @return a channel or @c NULL on error
 */
struct cio_channel *cio_channel_create(size_t item_size)
{
	cio_trace(__func__);

	struct cio_channel *c = calloc(1, sizeof (struct cio_channel));
	if (c == NULL)
		return NULL;

	c->refs = 1;
	c->item_size = item_size;

	return c;
}

/**
 * Increase the reference count.
 *
 * @param c
 *
 * @return @p c
 */
struct cio_channel *cio_channel_ref(struct cio_channel *c)
{
	cio_trace(__func__);

	cio_increment(&c->refs);
	return c;
}

/**
 * Decrease the reference count.  The channel will be destroyed when there are
 * no more references.
 */
void cio_channel_unref(struct cio_channel *c)
{
	cio_trace(__func__);

	if (cio_decrement(&c->refs) == 0)
		free(c);
}

size_t cio_channel_item_size(const struct cio_channel *c)
{
	return c->item_size;
}

/**
 * Mark the channel as closed.  Writing is not possible immediately after this,
 * and reading is not possible after all pending writes have been completed.
 * This function may be called multiple times for a single channel.
 */
void cio_channel_close(struct cio_channel *c)
{
	cio_trace(__func__);

	if (c->closed)
		return;

	c->closed = true;

	struct cio_list list = c->read_list;

	c->read_list.head = NULL;
	c->read_list.tail = NULL;

	/* TODO: postpone reader scheduling and return immediately (this messes up Python state) */
	while (true) {
		struct cio_channel_wait *read = cio_channel_wait_head(&list);
		if (read == NULL)
			break;

		read->closed = true;
		cio_run(read->context, read->id);
	}
}

/**
 * Read an item from the channel.
 *
 * @param c
 * @param item       buffer for the item or @c NULL if @p item_size is 0
 * @param item_size  must match the channel's item size
 *
 * @retval 1 on success
 * @retval 0 if the channel is closed
 * @retval -1 on error with @c errno set
 */
int cio_channel_read(struct cio_channel *c, void *item, size_t item_size)
{
	cio_trace(__func__);

	if (cio_channel_check(c, item_size) < 0)
		return -1;

	struct cio_channel_wait *write = cio_channel_wait_head(&c->write_list);
	if (write) {
		memcpy(item, write->item, item_size);
		cio_run(write->context, write->id);
		return 1;
	} else {
		if (c->closed)
			return 0;

		return cio_channel_wait(&c->read_list, item);
	}
}

/**
 * Write an item to the channel.
 *
 * @param c
 * @param item       the item or @c NULL if @p item_size is 0
 * @param item_size  must match the channel's item size
 *
 * @retval 1 on success
 * @retval 0 if the channel is closed
 * @retval -1 on error with @c errno set
 */
int cio_channel_write(struct cio_channel *c, const void *item, size_t item_size)
{
	cio_trace(__func__);

	if (cio_channel_check(c, item_size) < 0)
		return -1;

	if (c->closed)
		return 0;

	struct cio_channel_wait *read = cio_channel_wait_head(&c->read_list);
	if (read) {
		memcpy(read->item, item, item_size);
		cio_run(read->context, read->id);
		return 1;
	} else {
		return cio_channel_wait(&c->write_list, (void *) item);
	}
}

/**
 * Perform one of multiple read and/or write operations.
 *
 * @param      ops        vector of operations
 * @param      nops       number of operations
 * @param[out] selection  storage for the index of the selected operation
 *                        (set also on error; -1 on general error)
 *
 * @retval 1 if the selected operation performed
 * @retval 0 if the selected channel is closed
 * @retval -1 on error with @c errno set
 */
int cio_channel_select(const struct cio_channel_op *ops, int nops, int *selection)
{
	cio_trace(__func__);

	for (int i = 0; i < nops; i++) {
		const struct cio_channel_op *op = ops + i;
		struct cio_channel_wait *write = cio_channel_wait_head(&op->channel->write_list);
		struct cio_channel_wait *read = cio_channel_wait_head(&op->channel->read_list);

		*selection = i;

		if (cio_channel_check(op->channel, op->item_size) < 0)
			return -1;

		if (op->type == CIO_CHANNEL_READ && write) {
			memcpy(op->item, write->item, op->item_size);
			cio_run(write->context, write->id);
			return 1;
		}

		if (op->channel->closed)
			return 0;

		if (op->type == CIO_CHANNEL_WRITE && read) {
			memcpy(read->item, op->item, op->item_size);
			cio_run(read->context, read->id);
			return 1;
		}
	}

	struct cio_context context;
	struct cio_channel_wait nodes[nops];

	cio_tracef("%s: alloc context %p", __func__, &context);

	for (int i = 0; i < nops; i++) {
		const struct cio_channel_op *op = ops + i;
		cio_channel_wait_append(cio_channel_wait_list(op), nodes + i, i + 1, op->item, &context);
	}

	int ret = cio_yield(&context);

	if (ret < 0)
		*selection = -1;
	else
		*selection = ret - 1;

	for (int i = 0; i < nops; i++) {
		const struct cio_channel_op *op = ops + i;
		cio_channel_wait_remove_head(cio_channel_wait_list(op));
	}

	cio_tracef("%s: free context %p", __func__, &context);

	if (ret < 0)
		return -1;

	if (nodes[*selection].closed)
		return 0;

	return 1;
}

/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_CHANNEL_H
#define CIO_CHANNEL_H

/**
 * @defgroup channel Channel
 * @code #include <cio/channel.h> @endcode
 * @{
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct cio_channel
 * @brief Opaque channel object.
 */
struct cio_channel;

enum cio_channel_op_type {
	/**
	 * Indicates a read operation.
	 */
	CIO_CHANNEL_READ,

	/**
	 * Indicates a write operation.
	 */
	CIO_CHANNEL_WRITE,
};

/**
 * @brief Description of a multiplexed channel operation.
 */
struct cio_channel_op {
	/**
	 * @brief Operation type.
	 */
	enum cio_channel_op_type type;

	/**
	 * @brief Target channel.
	 */
	struct cio_channel *channel;

	/**
	 * @brief Buffer to read to or write from.
	 */
	void *item;

	/**
	 * @brief Buffer size.
	 */
	size_t item_size;
};

struct cio_channel *cio_channel_create(size_t item_size);
struct cio_channel *cio_channel_ref(struct cio_channel *);
void cio_channel_unref(struct cio_channel *);
size_t cio_channel_item_size(const struct cio_channel *);
void cio_channel_close(struct cio_channel *);
int cio_channel_read(struct cio_channel *, void *item, size_t item_size);
int cio_channel_write(struct cio_channel *, const void *item, size_t item_size);
int cio_channel_select(const struct cio_channel_op *ops, int nops, int *index_out);

#ifdef __cplusplus
}
#endif

#endif
/** @} */

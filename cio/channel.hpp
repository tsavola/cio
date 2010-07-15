/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_CHANNEL_HPP
#define CIO_CHANNEL_HPP

#include "channel.h"

#include <cassert>
#include <stdexcept>

namespace cio {

/**
 * @defgroup channel_cpp Channel (C++)
 * @code #include <cio/channel.hpp> @endcode
 * @{
 */

template <typename T>
class channel
{
	struct cio_channel *c;

public:
	/**
	 * Create a new channel.
	 */
	channel() throw (std::bad_alloc) : c(cio_channel_create(sizeof (T)))
	{
		if (!c)
			throw std::bad_alloc();
	}

	/**
	 * Steal a reference to a C channel object.
	 *
	 * @pre Item size of @p c_ptr must match @b T
	 */
	explicit channel(struct cio_channel *c_ptr) throw () : c(c_ptr)
	{
		assert(cio_channel_item_size(c_ptr) == sizeof (T));
	}

	/**
	 * Reference an existing channel.
	 */
	channel(channel &other) throw () : c(other.c)
	{
		cio_channel_ref(c);
	}

	/**
	 * Unreference the channel.
	 */
	~channel() throw ()
	{
		cio_channel_unref(c);
	}

	/**
	 * Drop the existing reference and reference another channel.
	 */
	channel &operator=(channel &other) throw ()
	{
		cio_channel_unref(c);
		c = other.c;
		cio_channel_ref(c);
		return *this;
	}

	/**
	 * @see cio_channel_close
	 */
	void close() throw ()
	{
		cio_channel_close(c);
	}

	/**
	 * @see cio_read
	 */
	int read(T &item) throw ()
	{
		return cio_channel_read(c, reinterpret_cast<void *> (&item), sizeof (T));
	}

	/**
	 * @see cio_write
	 */
	int write(const T &item) throw ()
	{
		return cio_channel_write(c, reinterpret_cast<const void *> (&item), sizeof (T));
	}

	/**
	 * @see channel::read(T &)
	 */
	channel &operator>>(T &item) throw ()
	{
		int ret = read(item);
		assert(ret == 1);
		return *this;
	}

	/**
	 * @see channel::write(const T &)
	 */
	channel &operator<<(const T &item) throw ()
	{
		int ret = write(item);
		assert(ret == 1);
		return *this;
	}

	/**
	 * @return a shared reference to the C channel object
	 */
	struct cio_channel *c_ptr() throw ()
	{
		return c;
	}
};

/** @} */

} // namespace cio

#endif

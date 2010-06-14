/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_CHANNEL_HPP
#define CIO_CHANNEL_HPP

/**
 * @defgroup channel_cpp Channel (C++)
 * @code #include <cio/channel.hpp> @endcode
 * @{
 */

#include "channel.h"

#include <cassert>
#include <stdexcept>

namespace cio {

/**
 * TODO
 */
template <typename T>
class channel
{
	struct cio_channel *c;

public:
	channel() throw (std::bad_alloc) : c(cio_channel_create(sizeof (T)))
	{
		if (!c)
			throw std::bad_alloc();
	}

	/**
	 * TODO
	 */
	explicit channel(struct cio_channel *c_ptr) throw () : c(c_ptr)
	{
		assert(cio_channel_item_size(c) == sizeof (T));
	}

	channel(channel &other) throw () : c(other.c)
	{
		cio_channel_ref(c);
	}

	~channel() throw ()
	{
		cio_channel_unref(c);
	}

	channel &operator=(channel &other) throw ()
	{
		cio_channel_unref(c);
		c = other.c;
		cio_channel_ref(c);
		return *this;
	}

	/**
	 * TODO
	 */
	int read(T &item) throw ()
	{
		return cio_channel_read(c, reinterpret_cast<void *> (&item), sizeof (T));
	}

	/**
	 * TODO
	 */
	int write(const T &item) throw ()
	{
		return cio_channel_write(c, reinterpret_cast<const void *> (&item), sizeof (T));
	}

	/**
	 * TODO
	 */
	channel &operator>>(T &item) throw ()
	{
		int ret = read(item);
		assert(ret == 1);
		return *this;
	}

	/**
	 * TODO
	 */
	channel &operator<<(const T &item) throw ()
	{
		int ret = write(item);
		assert(ret == 1);
		return *this;
	}

	/**
	 * TODO
	 */
	struct cio_channel *c_ptr() throw ()
	{
		return c;
	}
};

} // namespace cio

/** @} */

#endif

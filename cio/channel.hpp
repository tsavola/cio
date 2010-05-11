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

#include <stdexcept>

#include "error.hpp"

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
	channel(struct cio_channel *c_ptr) throw () : c(c_ptr)
	{
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
	channel &operator>>(T &item) throw (error)
	{
		int ret = cio_channel_read(c, reinterpret_cast<void *> (&item), sizeof (T));
		if (ret < 0)
			throw error(ret);
		return *this;
	}

	/**
	 * TODO
	 */
	channel &operator<<(const T &item) throw (error)
	{
		int ret = cio_channel_write(c, reinterpret_cast<const void *> (&item), sizeof (T));
		if (ret < 0)
			throw error(ret);
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

/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_CHANNEL_HPP
#define CIO_CHANNEL_HPP

#include "channel.h"

#include <cassert>
#include <cstring>
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
	bool read(T &item)
	{
		int ret = cio_channel_read(c, reinterpret_cast<void *> (&item), sizeof (T));
		if (ret < 0)
			throw std::runtime_error(std::strerror(errno));
		return ret != 0;
	}

	/**
	 * @see cio_write
	 */
	bool write(const T &item)
	{
		int ret = cio_channel_write(c, reinterpret_cast<const void *> (&item), sizeof (T));
		if (ret < 0)
			throw std::runtime_error(std::strerror(errno));
		return ret != 0;
	}

	/**
	 * @return a shared reference to the C channel object
	 */
	struct cio_channel *c_ptr() throw ()
	{
		return c;
	}

	class iterator
	{
		friend class channel;

		channel *c;
		mutable T item;

		explicit iterator(channel *c_) : c(c_)
		{
			operator++();
		}

		iterator() throw () : c(0)
		{
		}

	public:
		T operator*() const throw ()
		{
			return item;
		}

		T *operator->() const throw ()
		{
			return &item;
		}

		T &operator++()
		{
			if (!c->read(item))
				c = 0;
			return item;
		}

		T operator++(int)
		{
			T copy = item;
			if (!c->read(item))
				c = 0;
			return copy;
		}

		bool operator==(const iterator &other) const throw ()
		{
			return !c && !other.c;
		}

		bool operator!=(const iterator &other) const throw ()
		{
			return !operator==(other);
		}
	};

	iterator begin()
	{
		return iterator(this);
	}

	iterator end() throw ()
	{
		return iterator();
	}
};

/** @} */

} // namespace cio

#endif

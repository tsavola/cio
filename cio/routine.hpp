/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_ROUTINE_HPP
#define CIO_ROUTINE_HPP

/**
 * @defgroup routine_cpp Routine (C++)
 * @code #include <cio/routine.hpp> @endcode
 * @{
 */

#include <cerrno>
#include <type_traits>

#include "error.h"
#include "routine.h"

#include "error.hpp"

namespace cio {

/**
 * @internal
 */
template <typename T, bool is_pod>
struct launch_arg;

template <typename T>
struct launch_arg<T, true> {
	static inline int init(void *buf, void *orig, size_t) throw ()
	{
		*reinterpret_cast<T *> (buf) = *reinterpret_cast<T *> (orig);
		return 0;
	}

	static inline void destroy(T *) throw ()
	{
	}
};

template <typename T>
struct launch_arg<T, false> {
	static inline int init(void *buf, void *orig, size_t) throw ()
	{
		try {
			new (buf) T(*reinterpret_cast<T *> (orig));
			return 0;
		} catch (...) {
			cio_error("Exception thrown in routine argument copy constructor", 0);
			errno = EINVAL;
			return -1;
		}
	}

	static inline void destroy(T *ptr) throw ()
	{
		try {
			ptr->~T();
		} catch (...) {
			cio_error("Exception thrown in routine argument destructor", 0);
		}
	}
};

/**
 * @internal
 */
template <typename T>
void launch_call(void (*func)(void *), void *buf) throw ()
{
	T *arg = reinterpret_cast<T *> (buf);
	try {
		((void (*)(T &)) func)(*arg);
	} catch (...) {
		cio_error("Uncaught exception in routine", 0);
	}
	launch_arg<T, std::is_pod<T>::value>::destroy(arg);
}

/**
 * @see cio_launch()
 */
template <typename T>
inline void launch(void (*func)(T &), T &arg) throw (error)
{
	int ret = cio_launch5((void (*)(void *)) func, &arg, sizeof (T), launch_arg<T, std::is_pod<T>::value>::init, launch_call<T>);
	if (ret < 0)
		throw error(ret);
}

} // namespace cio

/** @} */

#endif

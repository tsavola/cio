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

#include <stdexcept>
#include <type_traits>

#include "error.h"
#include "routine.h"

namespace cio {

/**
 * @internal
 */
template <typename T, bool is_pod>
struct launch_arg;

template <typename T>
struct launch_arg<T, true> {
	inline void init(void *stack, T &arg) throw ()
	{
		*reinterpret_cast<T *> (stack) = arg;
	}

	static inline void destroy(T *) throw ()
	{
	}
};

template <typename T>
struct launch_arg<T, false> {
	void *orphan;

	inline void init(void *stack, T &arg)
	{
		orphan = stack;
		new (stack) T(arg);
		orphan = 0;
	}

	inline ~launch_arg() throw ()
	{
		if (orphan) {
			destroy(reinterpret_cast<T *> (orphan));
			cio_launch_cancel(orphan, sizeof (T));
		}
	}

	static inline void destroy(T *ptr) throw ()
	{
		try {
			ptr->~T();
		} catch (...) {
			cio_error("Exception thrown in routine argument destructor");
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
		cio_error("Uncaught exception in routine");
	}
	launch_arg<T, std::is_pod<T>::value>::destroy(arg);
}

/**
 * @see cio_launch()
 */
template <typename T>
void launch(void (*func)(T &), T &arg) throw (std::bad_alloc)
{
	void *stack = cio_launch_prepare(sizeof (T));
	if (!stack)
		throw std::bad_alloc();

	launch_arg<T, std::is_pod<T>::value> sentinel;
	sentinel.init(stack, arg);

	cio_launch_finish(stack, (void (*)(void *)) func, sizeof (T), launch_call<T>);
}

} // namespace cio

/** @} */

#endif

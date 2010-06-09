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
	inline void init(void *target_arg, T &source_arg) throw ()
	{
		*reinterpret_cast<T *> (target_arg) = source_arg;
	}

	static inline void destroy(T *) throw ()
	{
	}
};

template <typename T>
struct launch_arg<T, false> {
	void *orphan_arg;

	inline void init(void *target_arg, T &source_arg)
	{
		orphan_arg = target_arg;
		new (target_arg) T(source_arg);
		orphan_arg = 0;
	}

	inline ~launch_arg() throw ()
	{
		if (orphan_arg) {
			destroy(reinterpret_cast<T *> (orphan_arg));
			cio_launch_cancel(orphan_arg);
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
void CIO_NORETURN launch_call(void (*func)(void *), void *void_arg) throw ()
{
	T *type_arg = reinterpret_cast<T *> (void_arg);
	try {
		((void (*)(T &)) func)(*type_arg);
	} catch (...) {
		cio_error("Uncaught exception in routine");
	}
	launch_arg<T, std::is_pod<T>::value>::destroy(type_arg);
	cio_launch_exit(void_arg);
}

/**
 * @see cio_launch()
 */
template <typename T>
void launch(void (*func)(T &), T &arg) throw (std::bad_alloc)
{
	void *routine_arg = cio_launch_prepare((void (*)(void *)) func, sizeof (T), launch_call<T>);
	if (!routine_arg)
		throw std::bad_alloc();

	launch_arg<T, std::is_pod<T>::value> sentinel;
	sentinel.init(routine_arg, arg);

	cio_launch_finish(routine_arg);
}

} // namespace cio

/** @} */

#endif

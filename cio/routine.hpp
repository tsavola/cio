/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_ROUTINE_HPP
#define CIO_ROUTINE_HPP

#include <stdexcept>
#include <type_traits>

#include "error.h"
#include "routine.h"

namespace cio {

/**
 * @defgroup routine_cpp Routine (C++)
 * @code #include <cio/routine.hpp> @endcode
 * @{
 */

namespace internal {
	template <typename T, bool is_pod>
	struct routine_arg;

	template <typename T>
	struct routine_arg<T, true> {
		inline void init(void *target_arg, T &source_arg) throw ()
		{
			*reinterpret_cast<T *> (target_arg) = source_arg;
		}

		static inline void destroy(T *) throw ()
		{
		}
	};

	template <typename T>
	struct routine_arg<T, false> {
		void *orphan_arg;

		inline void init(void *target_arg, T &source_arg)
		{
			orphan_arg = target_arg;
			new (target_arg) T(source_arg);
			orphan_arg = 0;
		}

		inline ~routine_arg() throw ()
		{
			if (orphan_arg) {
				destroy(reinterpret_cast<T *> (orphan_arg));
				cio_routine_cancel(orphan_arg);
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

	template <typename T>
	void CIO_NORETURN routine_call(void (*func)(void *), void *void_arg) throw ()
	{
		T *type_arg = reinterpret_cast<T *> (void_arg);
		try {
			((void (*)(T &)) func)(*type_arg);
		} catch (...) {
			cio_error("Uncaught exception in routine");
		}
		routine_arg<T, std::is_pod<T>::value>::destroy(type_arg);
		cio_routine_exit(void_arg);
	}
}

/**
 * @see cio_routine()
 */
template <typename T>
void routine(void (*func)(T &), T &arg) throw (std::bad_alloc)
{
	void *arg_copy = cio_routine_prepare((void (*)(void *)) func, sizeof (T), internal::routine_call<T>);
	if (!arg_copy)
		throw std::bad_alloc();

	internal::routine_arg<T, std::is_pod<T>::value> sentinel;
	sentinel.init(arg_copy, arg);

	cio_routine_finish(arg_copy);
}

/** @} */

} // namespace cio

#endif

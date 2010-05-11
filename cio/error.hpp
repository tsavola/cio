/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_ERROR_HPP
#define CIO_ERROR_HPP

/**
 * @defgroup error_cpp Error (C++)
 * @code #include <cio/error.hpp> @endcode
 * @{
 */

#include <cstring>
#include <exception>

namespace cio {

/**
 * TODO
 */
class error : public std::exception
{
	const int number_;

public:
	error(int number) throw () : number_(number)
	{
	}

	/**
	 * TODO
	 */
	virtual int number() const throw ()
	{
		return number_;
	}

	virtual const char *what() const throw ()
	{
		return std::strerror(number_);
	}
};

} // namespace cio

/** @} */

#endif

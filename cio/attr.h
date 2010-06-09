/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_ATTR_H
#define CIO_ATTR_H

#ifdef __GNUC__
# define CIO_INTERNAL   __attribute__ ((visibility ("hidden")))
# define CIO_NORETURN   __attribute__ ((noreturn))
#else
# define CIO_INTERNAL
# define CIO_NORETURN
#endif

#endif

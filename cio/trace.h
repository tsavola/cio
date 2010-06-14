/*
 * Copyright (c) 2010  Timo Savola
 */

#ifdef cio_trace
# undef cio_trace
# undef cio_tracef
#endif

#ifdef CIO_TRACE

# include <errno.h>
# include <stdio.h>

#define cio_trace(message) \
	cio_tracef("%s", message)

# define cio_tracef(format, args...) \
	do { \
		int saved_errno = errno; \
		fprintf(stderr, "TRACE: " format "\n", args); \
		errno = saved_errno; \
	} while (0)

#else

# define cio_trace(message)
# define cio_tracef(format, args...)

#endif

/*
 * Copyright (c) 2010  Timo Savola
 */

#ifdef cio_trace
# undef cio_trace
# undef cio_tracef
#endif

#ifdef CIO_TRACE
# include <stdio.h>
# define cio_tracef(format, args...)  fprintf(stderr, "TRACE: " format "\n", args)
# define cio_trace(message)           cio_tracef("%s", message)
#else
# define cio_tracef(format, args...)
# define cio_trace(message)
#endif

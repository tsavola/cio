/*
 * Copyright (c) 2010  Timo Savola
 */

#include "error-internal.h"
#include "error.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PREFIX  "cio: "

/**
 * @internal
 */
void cio_error(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	fprintf(stderr, PREFIX);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

/**
 * @internal
 */
void CIO_NORETURN cio_abort(const char *message, int error)
{
	if (error)
		cio_error("%s: %s", message, strerror(error));
	else
		cio_error("%s", message);

	abort();
}

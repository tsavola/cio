/*
 * Copyright (c) 2010  Timo Savola
 */

#include "error-internal.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PREFIX  "cio: "

/**
 * @internal
 */
void cio_error(const char *message, int error)
{
	if (error)
		fprintf(stderr, PREFIX "%s: %s\n", message, strerror(error));
	else
		fprintf(stderr, PREFIX "%s\n", message);
}

/**
 * @internal
 */
void CIO_NORETURN cio_abort(const char *message, int error)
{
	cio_error(message, error);
	abort();
}

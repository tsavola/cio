/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_ERROR_INTERNAL_H
#define CIO_ERROR_INTERNAL_H

#include "attr.h"

void CIO_NORETURN cio_abort(const char *message, int error);

#endif

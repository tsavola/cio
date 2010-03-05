/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_ROUTINE_INTERNAL_H
#define CIO_ROUTINE_INTERNAL_H

#include "attr-internal.h"

void CIO_INTERNAL *cio_cleanup_stack(void);
void CIO_INTERNAL CIO_NORETURN cio_cleanup(void *stack);

#endif

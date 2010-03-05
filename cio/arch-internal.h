/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_ARCH_INTERNAL_H
#define CIO_ARCH_INTERNAL_H

#include "attr-internal.h"

void CIO_INTERNAL CIO_NORETURN cio_start(void *argument, void (*routine)(void *), void *stack);

#endif

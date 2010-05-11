/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_ARCH_INTERNAL_H
#define CIO_ARCH_INTERNAL_H

#include "attr-internal.h"

void CIO_INTERNAL CIO_NORETURN cio_start(void (*func)(void *), void *arg, void *stack, void (*call)(void (*)(void *), void *));

#endif

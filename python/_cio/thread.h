/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef PY_CIO_THREAD_H
#define PY_CIO_THREAD_H

#include "cio/attr.h"

void py_cio_thread_save(void) CIO_INTERNAL;
void py_cio_thread_restore(void) CIO_INTERNAL;

#endif

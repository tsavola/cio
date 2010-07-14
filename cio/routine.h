/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_ROUTINE_H
#define CIO_ROUTINE_H

/**
 * @defgroup routine Routine
 * @code #include <cio/routine.h> @endcode
 * @{
 */

#include <stddef.h>

#include <ucontext.h>

#include "attr.h"

#ifdef __cplusplus
extern "C" {
#endif

int cio_routine(void (*routine)(void *), const void *arg, size_t argsize);

void *cio_routine_prepare(void (*func)(void *), size_t argsize, void CIO_NORETURN (*call)(void (*)(void *), void *));
void cio_routine_finish(void *);
void cio_routine_cancel(void *);
void CIO_NORETURN cio_routine_exit(void *);

#ifdef __cplusplus
}
#endif

/** @} */

#endif

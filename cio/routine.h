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

#ifdef __cplusplus
extern "C" {
#endif

int cio_launch(void (*routine)(void *), const void *arg, size_t argsize);
int cio_launch5(void (*routine)(void *), void *arg, size_t argsize, int (*init)(void *, void *, size_t), void (*call)(void (*)(void *), void *));

#ifdef __cplusplus
}
#endif

/** @} */

#endif

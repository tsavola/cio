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

#ifdef __cplusplus
}
#endif

/** @} */

#endif

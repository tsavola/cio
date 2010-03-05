/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_TIME_H
#define CIO_TIME_H

/**
 * @defgroup time Time
 * @code #include <cio/time.h> @endcode
 * @{
 */

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int cio_sleep(const struct timespec *interval);

#ifdef __cplusplus
}
#endif

/** @} */

#endif

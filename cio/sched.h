/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_SCHED_H
#define CIO_SCHED_H

/**
 * @defgroup sched Scheduling
 * @code #include <cio/sched.h> @endcode
 * @{
 */

#include <ucontext.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief File descriptor may be read from.
 */
#define CIO_INPUT  1

/**
 * @brief File descriptor may be written to.
 */
#define CIO_OUTPUT 2

/**
 * @brief Error while waiting for I/O.
 */
#define CIO_ERROR 4

/**
 * @brief Hang-up while waiting for I/O.
 */
#define CIO_HANGUP 8

/**
 * @brief Buffer for saving stack and register state.
 */
struct cio_context {
	ucontext_t ucontext;
	int value;
	void *cleanup;
};

int cio_register(int fd, int events, struct cio_context *target);
void cio_unregister(int fd);
int cio_yield(struct cio_context *storage);
void cio_run(struct cio_context *target, int value);

#ifdef __cplusplus
}
#endif

/** @} */

#endif

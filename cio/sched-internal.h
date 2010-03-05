/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_SCHED_INTERNAL_H
#define CIO_SCHED_INTERNAL_H

#include <setjmp.h>

#include "attr-internal.h"
#include "sched.h"

/**
 * Node in the scheduler's runnable list.
 */
struct cio_runnable {
	/**
	 * Execution context which will be resumed.
	 */
	struct cio_context context;

	/**
	 * Used by struct cio_list.
	 */
	struct cio_runnable *next;
};

/**
 * Records the current execution context.  Returns twice: immediately after
 * saving and when the execution is resumed.
 *
 * @note The calling function must not return before the saved context is
 *       resumed.
 *
 * @param storage  struct cio_context pointer
 *
 * @retval 0 when the context is recorded
 * @retval non-0 when the execution is resumed
 */
#define cio_save(storage) \
	setjmp((storage)->env)

void CIO_INTERNAL cio_runnable(struct cio_runnable *node);
void CIO_INTERNAL CIO_NORETURN cio_sched(void);

#endif

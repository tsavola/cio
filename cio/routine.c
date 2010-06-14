/*
 * Copyright (c) 2010  Timo Savola
 */

#include "routine-internal.h"
#include "routine.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <ucontext.h>
#include <unistd.h>
#include <sys/mman.h>

#include "error-internal.h"
#include "error.h"
#include "sched-internal.h"
#include "trace.h"

#define STACK_SIZE          0x800000
#define GUARD_SIZE          0x1000

struct cio_routine {
	ucontext_t ucontext;
	char arg[0];
};

static struct cio_routine *cio_arg_routine(void *arg)
{
	return arg - sizeof (struct cio_routine);
}

static void *cio_stack_alloc(size_t size, size_t guard_size)
{
	void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
	if (ptr == MAP_FAILED)
		return NULL;

	if (mprotect(ptr, guard_size, PROT_NONE) < 0) {
		munmap(ptr, size);
		return NULL;
	}

	return ptr;
}

static void cio_stack_free(void *stack, size_t size)
{
	munmap(stack, size);
}

static void CIO_NORETURN cio_launch_call(void (*func)(void *), void *arg)
{
	func(arg);
	cio_launch_exit(arg);
}

/**
 * Create and schedule a new routine.  The routine will receive a copy of the
 * argument, allocated on its own stack.
 *
 * @param func     a function to execute
 * @param arg      pointer to an argument for to the routine
 * @param argsize  size of the argument
 *
 * @retval 0 on success
 * @retval -1 on error with @c errno set
 */
int cio_launch(void (*func)(void *), const void *arg, size_t argsize)
{
	void *routine_arg = cio_launch_prepare(func, argsize, cio_launch_call);
	if (routine_arg == NULL)
		return -1;

	memcpy(routine_arg, arg, argsize);
	cio_launch_finish(routine_arg);
	return 0;
}

/**
 * @internal
 */
void *cio_launch_prepare(void (*func)(void *), size_t argsize, void CIO_NORETURN (*call)(void (*)(void *), void *))
{
	struct cio_routine *routine = malloc(sizeof (struct cio_routine) + argsize);
	if (routine == NULL)
		goto no_routine;

	void *stack = cio_stack_alloc(STACK_SIZE, GUARD_SIZE);
	if (stack == NULL)
		goto no_stack;

	if (getcontext(&routine->ucontext) < 0)
		goto no_context;

	routine->ucontext.uc_stack.ss_sp = stack;
	routine->ucontext.uc_stack.ss_size = STACK_SIZE;

	/* TODO: pointer arguments are not portable */
	makecontext(&routine->ucontext, (void (*)(void)) call, 2, func, routine->arg);

	return routine->arg;

no_context:
	cio_stack_free(stack, STACK_SIZE);
no_stack:
	free(routine);
no_routine:
	return NULL;
}

/**
 * @internal
 */
void cio_launch_finish(void *arg)
{
	struct cio_routine *routine = cio_arg_routine(arg);
	struct cio_runnable node;

	cio_tracef("%s: alloc runnable %p", __func__, &node);

	if (cio_save(&node.context) == 0) {
		cio_runnable(&node);

		cio_tracef("%s: routine %p", __func__, routine);

		setcontext(&routine->ucontext);
		cio_abort("Failed to launch routine", errno);
	}

	cio_tracef("%s: free runnable %p", __func__, &node);
}

/**
 * @internal
 */
void cio_launch_cancel(void *arg)
{
	cio_cleanup(arg);
}

/**
 * @internal
 */
void CIO_NORETURN cio_launch_exit(void *arg)
{
	while (true) {
		cio_sched(arg);

		/* TODO: this should be avoided by scheduling a waiting routine */
		cio_error("Warning: EINTR ignored");
	}
}

void CIO_INTERNAL cio_cleanup(void *arg)
{
	struct cio_routine *routine = cio_arg_routine(arg);

	cio_stack_free(routine->ucontext.uc_stack.ss_sp, routine->ucontext.uc_stack.ss_size);
	free(routine);
}

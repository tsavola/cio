/*
 * Copyright (c) 2010  Timo Savola
 */

#include "routine-internal.h"
#include "routine.h"

#include <errno.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>

#include "arch-internal.h"
#include "error-internal.h"
#include "sched-internal.h"

#define STACK_SIZE          0x800000
#define GUARD_SIZE          0x1000

#define CLEANUP_STACK_SIZE  0x2000
#define CLEANUP_GUARD_SIZE  0x1000

static pthread_key_t cio_cleanup_key;

static void *cio_stack_alloc(size_t size, size_t guard_size)
{
	void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
	if (ptr == MAP_FAILED)
		return NULL;

	if (mprotect(ptr, guard_size, PROT_NONE) < 0) {
		munmap(ptr, size);
		return NULL;
	}

	return ptr + size;
}

static void cio_stack_free(void *stack, size_t size)
{
	munmap(stack - size, size);
}

static size_t cio_alignment(size_t size)
{
	return (size + sizeof (long) - 1) & ~(size_t) (sizeof (long) - 1);
}

static void cio_launch_call(void (*func)(void *), void *arg)
{
	func(arg);
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
	void *stack = cio_launch_prepare(argsize);
	if (stack == NULL)
		return -1;

	memcpy(stack, arg, argsize);
	cio_launch_finish(stack, func, argsize, cio_launch_call);
	return 0;
}

/**
 * @internal
 */
void *cio_launch_prepare(size_t argsize)
{
	if (argsize > STACK_SIZE - GUARD_SIZE * 2) {
		errno = ENOMEM;
		return NULL;
	}

	void *stack_top = cio_stack_alloc(STACK_SIZE, GUARD_SIZE);
	if (stack_top == NULL)
		return NULL;

	return stack_top - cio_alignment(argsize);
}

/**
 * @internal
 */
void cio_launch_finish(void *stack, void (*func)(void *), size_t argsize, void (*call)(void (*)(void *), void *))
{
	void *stackarg = stack;

	void *stack_top = stack + cio_alignment(argsize);
	stack -= sizeof (void *);
	*(void **) stack = stack_top;

	struct cio_runnable node;

	if (cio_save(&node.context) == 0) {
		cio_runnable(&node);
		cio_start(func, stackarg, stack, call);
	}
}

/**
 * @internal
 */
void cio_launch_cancel(void *stack, size_t argsize)
{
	void *stack_top = stack + cio_alignment(argsize);
	cio_stack_free(stack_top, STACK_SIZE);
}

/**
 * Free the resources of an exited routine and schedule another one.  This
 * function is called with the cleanup stack as the active stack, so everything
 * must fit into that.
 *
 * @param stack  the stack pointer of the routine
 *
 * @internal
 */
void CIO_INTERNAL CIO_NORETURN cio_cleanup(void *stack)
{
	void *stack_top = *(void **) stack;
	cio_stack_free(stack_top, STACK_SIZE);

	cio_sched();
}

static void cio_cleanup_key_destroy(void *stack)
{
	cio_stack_free(stack, CLEANUP_STACK_SIZE);
}

static void cio_cleanup_key_create(void)
{
	pthread_key_create(&cio_cleanup_key, cio_cleanup_key_destroy);
}

/**
 * Get the cleanup stack pointer.
 *
 * @internal
 */
void CIO_INTERNAL *cio_cleanup_stack(void)
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;

	pthread_once(&once, cio_cleanup_key_create);

	void *stack = pthread_getspecific(cio_cleanup_key);
	if (stack == NULL) {
		stack = cio_stack_alloc(CLEANUP_STACK_SIZE, CLEANUP_GUARD_SIZE);
		if (stack == NULL)
			cio_abort("Failed to allocate memory for cleanup stack", errno);

		pthread_setspecific(cio_cleanup_key, stack);
	}

	return stack;
}

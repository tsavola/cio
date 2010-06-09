/*
 * Copyright (c) 2010  Timo Savola
 */

#include "sched-internal.h"
#include "sched.h"

#include <errno.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include <pthread.h>
#include <unistd.h>

#include <sys/epoll.h>

#include "error-internal.h"
#include "list-internal.h"
#include "trace.h"

static int cio_event_fd = -1;
static struct cio_list cio_runnable_list;

static void CIO_NORETURN cio_resume(struct cio_context *target, int value)
{
	longjmp(target->env, value);
}

static int cio_events_to_epoll(int cio)
{
	int epoll = 0;
	if (cio & CIO_INPUT)  epoll |= EPOLLIN;
	if (cio & CIO_OUTPUT) epoll |= EPOLLOUT;
	return epoll;
}

static int cio_events_from_epoll(int epoll)
{
	int cio = 0;
	if (epoll & EPOLLIN)  cio |= CIO_INPUT;
	if (epoll & EPOLLOUT) cio |= CIO_OUTPUT;
	if (epoll & EPOLLERR) cio |= CIO_ERROR;
	if (epoll & EPOLLHUP) cio |= CIO_HANGUP;
	return cio;
}

static void cio_event_create(void)
{
	cio_event_fd = epoll_create1(EPOLL_CLOEXEC);
}

static void cio_event_init(void)
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;

	pthread_once(&once, cio_event_create);
}

static void cio_sched_runnable(void)
{
	struct cio_runnable *node = cio_list_head(struct cio_runnable, &cio_runnable_list);
	if (node) {
		cio_tracef("%s: resume runnable %p", __func__, node);

		cio_list_remove_head(struct cio_runnable, &cio_runnable_list);
		cio_resume(&node->context, 1);
	}
}

static void CIO_NORETURN cio_sched_event(void)
{
	struct epoll_event event;

	while (epoll_wait(cio_event_fd, &event, 1, -1) < 0)
		if (errno != EINTR)
			cio_abort("Unexpected error while waiting for events", errno);

	cio_tracef("%s: resume context %p", __func__, event.data.ptr);

	cio_resume(event.data.ptr, cio_events_from_epoll(event.events));
}

/**
 * Jump to the next routine.
 *
 * @internal
 */
void CIO_INTERNAL CIO_NORETURN cio_sched(void)
{
	cio_sched_runnable();
	cio_sched_event();
}

/**
 * Add an execution context to the runnable queue.  @c node->context should be
 * initialized with cio_save().
 *
 * @param node  added on the runnable list
 *
 * @internal
 */
void CIO_INTERNAL cio_runnable(struct cio_runnable *node)
{
	cio_list_append(struct cio_runnable, &cio_runnable_list, node);
}

/**
 * Subscribe to I/O events on a file descriptor.  @p target should be passed to
 * cio_yield() which resumes execution when one of the events occur.
 *
 * @param fd      a file descriptor which supports polling
 * @param events  bitwise OR of #CIO_INPUT and/or #CIO_OUTPUT
 * @param target  execution context storage
 *
 * @retval 0 on success
 * @retval -1 on error with @c errno set
 */
int cio_register(int fd, int events, struct cio_context *target)
{
	cio_event_init();

	struct epoll_event event = {
		.events = cio_events_to_epoll(events),
		.data.ptr = target,
	};

	return epoll_ctl(cio_event_fd, EPOLL_CTL_ADD, fd, &event);
}

/**
 * Unsubscribe to the I/O events.
 *
 * @param fd  a file descriptor which was previously registered
 */
void cio_unregister(int fd)
{
	epoll_ctl(cio_event_fd, EPOLL_CTL_DEL, fd, NULL);
}

/**
 * Save current execution context and schedule the next routine.  This function
 * returns after an I/O event subscribed to with cio_register() occurs or
 * another routine calls cio_run() with the execution context stored here.
 *
 * @param storage  for saving the stack and register state until
 *                 execution is resumed
 *
 * @return the I/O events (bitwise OR of #CIO_INPUT, #CIO_OUTPUT, #CIO_ERROR and/or
 *         #CIO_HANGUP) which caused the execution to resume if cio_register() was
 *         used; or
 * @return the value passed to cio_run() if it was used
 */
int cio_yield(struct cio_context *storage)
{
	cio_tracef("%s: save context %p", __func__, storage);

	int ret = cio_save(storage);

	if (ret == 0)
		cio_sched();

	return ret;
}

/**
 * Jump to another routine immediately.
 *
 * @param target  execution context to be resumed
 * @param value to be returned by the cio_yield() call in the target routine
 *        (must be non-zero)
 */
void cio_run(struct cio_context *target, int value)
{
	struct cio_runnable node;

	cio_tracef("%s: alloc runnable %p", __func__, &node);

	cio_tracef("%s: resume context %p", __func__, target);

	if (cio_save(&node.context) == 0) {
		cio_runnable(&node);
		cio_resume(target, value);
	}

	cio_tracef("%s: free runnable %p", __func__, &node);
}

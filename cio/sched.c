/*
 * Copyright (c) 2010  Timo Savola
 */

#include "file.h"
#include "sched-internal.h"
#include "sched.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include <pthread.h>
#include <ucontext.h>
#include <unistd.h>

#include <sys/epoll.h>

#include "error-internal.h"
#include "list-internal.h"
#include "map-internal.h"

struct cio_wait {
	struct cio_map_node map_node;
	struct cio_context context;
};

static int cio_event_fd = -1;
static struct cio_map cio_wait_map;
static struct cio_list cio_runnable_list;

static void CIO_NORETURN cio_resume(struct cio_context *target, int value, void *cleanup)
{
	target->value = value;
	target->cleanup = cleanup;

	setcontext(&target->ucontext);
	cio_abort("Failed to resume context", errno);
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

static int cio_wait_add(int fd, struct cio_wait *node)
{
	return cio_map_add(&cio_wait_map, fd, node);
}

static struct cio_wait *cio_wait_find(int fd)
{
	return cio_map_find(&cio_wait_map, fd);
}

static int cio_wait_remove(int fd)
{
	return cio_map_remove(&cio_wait_map, fd);
}

static void cio_sched_runnable(void *cleanup)
{
	struct cio_runnable *node = cio_list_head(struct cio_runnable, &cio_runnable_list);
	if (node) {
		cio_list_remove_head(struct cio_runnable, &cio_runnable_list);
		cio_resume(&node->context, 1, cleanup);
	}
}

static void cio_sched_event(void *cleanup)
{
	struct epoll_event event;
	struct cio_wait *wait;

	if (epoll_wait(cio_event_fd, &event, 1, -1) < 0) {
		if (errno == EINTR)
			return;

		cio_abort("Unexpected error while waiting for events", errno);
	}

	wait = event.data.ptr;
	cio_resume(&wait->context, cio_events_from_epoll(event.events), cleanup);
}

/**
 * Jump to the next routine.
 *
 * @internal
 */
void CIO_INTERNAL cio_sched(void *cleanup)
{
	cio_sched_runnable(cleanup);
	cio_sched_event(cleanup);
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
 * Wait for I/O events on a file descriptor.
 *
 * @param fd      a file descriptor which supports polling
 * @param events  bitwise OR of #CIO_INPUT and/or #CIO_OUTPUT
 *
 * @return the I/O events (bitwise OR of #CIO_INPUT, #CIO_OUTPUT, #CIO_ERROR,
 *         #CIO_HANGUP and/or #CIO_CLOSE) which caused the execution to resume
 * @retval -1 on error with @c errno set
 */
int cio_wait(int fd, int events)
{
	cio_event_init();

	struct cio_wait wait;
	if (cio_wait_add(fd, &wait) < 0)
		return -1;

	struct epoll_event event = {
		.events = cio_events_to_epoll(events),
		.data.ptr = &wait,
	};

	int revents = cio_save(&wait.context);

	if (revents == 0) {
		int ret = epoll_ctl(cio_event_fd, EPOLL_CTL_ADD, fd, &event);
		int err = errno;

		if (ret == 0) {
			cio_sched(NULL);
			ret = -1;
			err = errno;

			epoll_ctl(cio_event_fd, EPOLL_CTL_DEL, fd, NULL);
		}

		cio_wait_remove(fd);

		errno = err;
		return ret;
	} else {
		epoll_ctl(cio_event_fd, EPOLL_CTL_DEL, fd, NULL);
		cio_wait_remove(fd);

		return revents;
	}
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
 * @retval -1 on interrupt with @c errno set
 */
int cio_yield(struct cio_context *storage)
{
	cio_event_init();

	int ret = cio_save(storage);

	if (ret == 0) {
		cio_sched(NULL);
		return -1;
	}

	return ret;
}

/**
 * Jump to another routine immediately.
 *
 * @param target  execution context to be resumed
 * @param value to be returned by the cio_yield() call in the target routine
 *        (must be greater than zero)
 */
void cio_run(struct cio_context *target, int value)
{
	assert(value > 0);

	struct cio_runnable node;

	if (cio_save(&node.context) == 0) {
		cio_runnable(&node);
		cio_resume(target, value, NULL);
	}
}

/**
 * Close a file descriptor and interrupt routines which are waiting for I/O on
 * it.
 */
int cio_close(int fd)
{
	struct cio_wait *wait;

	wait = cio_wait_find(fd);
	if (wait) {
		cio_wait_remove(fd);
		cio_run(&wait->context, CIO_CLOSE);

		errno = EINPROGRESS;
		return -1;
	} else {
		return close(fd);
	}
}

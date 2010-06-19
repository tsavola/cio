/*
 * Copyright (c) 2010  Timo Savola
 */

#include <Python.h>

#include "socket.h"

#include <stddef.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "cio/socket.h"

#include "thread.h"

PyObject *py_cio_connect(PyObject *self, PyObject *args)
{
	int sockfd;
	const char *node;
	const char *service;
	struct addrinfo *infos;
	socklen_t len;
	int ret;
	int err;

	if (!PyArg_ParseTuple(args, "i(zs)", &sockfd, &node, &service))
		return NULL;

	py_cio_thread_save();

	struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
	};

	len = sizeof (hints.ai_socktype);
	ret = getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &hints.ai_socktype, &len);
	if (ret < 0) {
		py_cio_thread_restore();
		return PyErr_SetFromErrno(PyExc_IOError);
	}

	len = sizeof (hints.ai_protocol);
	ret = getsockopt(sockfd, SOL_SOCKET, SO_PROTOCOL, &hints.ai_protocol, &len);
	if (ret < 0) {
		py_cio_thread_restore();
		return PyErr_SetFromErrno(PyExc_IOError);
	}

	ret = getaddrinfo(node, service, &hints, &infos);
	if (ret) {
		py_cio_thread_restore();
		PyErr_SetString(PyExc_IOError, gai_strerror(ret));
		return NULL;
	}

	struct addrinfo *info;
	for (info = infos; info; info = info->ai_next) {
		ret = cio_connect(sockfd, info->ai_addr, info->ai_addrlen);
		if (ret < 0) {
			if (errno == EAFNOSUPPORT)
				continue;

			err = errno;
			break;
		}
	}

	if (ret < 0 && err == EINTR)
		PyErr_SetInterrupt();

	freeaddrinfo(infos);

	py_cio_thread_restore();

	if (ret < 0) {
		errno = err;
		return PyErr_SetFromErrno(PyExc_IOError);
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyObject *py_cio_accept(PyObject *self, PyObject *args)
{
	int sockfd;
	int ret;
	int err;

	if (!PyArg_ParseTuple(args, "i:accept", &sockfd))
		return NULL;

	py_cio_thread_save();

	ret = cio_accept(sockfd, NULL, NULL);
	err = errno;

	if (ret < 0 && err == EINTR)
		PyErr_SetInterrupt();

	py_cio_thread_restore();

	if (ret < 0) {
		errno = err;
		return PyErr_SetFromErrno(PyExc_IOError);
	}

	return PyLong_FromLong(ret);
}

/* py_cio_recv() and py_cio_send() are implemented in io.c */

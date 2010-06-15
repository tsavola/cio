/*
 * Copyright (c) 2010  Timo Savola
 */

#include <Python.h>

#include <stddef.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "cio/io-internal.h"
#include "cio/io.h"
#include "cio/routine.h"
#include "cio/socket.h"

static __thread PyThreadState *py_cio_thread_state;

static void py_cio_thread_save(void)
{
	py_cio_thread_state = PyEval_SaveThread();
}

static void py_cio_thread_restore(void)
{
	PyEval_RestoreThread(py_cio_thread_state);
}

static void py_cio_routine(void *arg)
{
	PyObject *callable = *(PyObject **) arg;
	PyObject *result;

	py_cio_thread_restore();

	result = PyObject_CallObject(callable, NULL);
	if (result == NULL)
		PyErr_Print();

	Py_XDECREF(result);
	Py_DECREF(callable);

	py_cio_thread_save();
}

static PyObject *py_cio_launch(PyObject *self, PyObject *args)
{
	PyObject *callable;
	int ret;

	if (!PyArg_ParseTuple(args, "O:launch", &callable))
		return NULL;

	if (!PyCallable_Check(callable)) {
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return NULL;
	}

	Py_INCREF(callable);

	py_cio_thread_save();
	ret = cio_launch(py_cio_routine, &callable, sizeof (PyObject *));
	py_cio_thread_restore();

	if (ret < 0) {
		Py_DECREF(callable);
		return PyErr_NoMemory();
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *py_cio_io(enum cio_io_type type, PyObject *args, const char *format)
{
	int fd;
	Py_buffer buf;
	Py_ssize_t size;
	int flags;
	ssize_t len;
	int err;

	if (!PyArg_ParseTuple(args, format, &fd, &buf, &size, &flags))
		return NULL;

	py_cio_thread_save();

	if (size > buf.len)
		size = buf.len;

	switch (type) {
	case CIO_IO_READ:
		len = cio_read(fd, buf.buf, size);
		break;

	case CIO_IO_WRITE:
		len = cio_write(fd, buf.buf, size);
		break;

	case CIO_IO_RECV:
		len = cio_recv(fd, buf.buf, size, flags);
		break;

	case CIO_IO_SEND:
		len = cio_send(fd, buf.buf, size, flags);
		break;
	}

	err = errno;

	if (len < 0 && errno == EINTR)
		PyErr_SetInterrupt();

	py_cio_thread_restore();

	PyBuffer_Release(&buf);

	if (len < 0) {
		errno = err;
		return PyErr_SetFromErrno(PyExc_IOError);
	}

	return PyLong_FromLong(len);
}

static PyObject *py_cio_read(PyObject *self, PyObject *args)
{
	return py_cio_io(CIO_IO_READ, args, "iw*n:read");
}

static PyObject *py_cio_write(PyObject *self, PyObject *args)
{
	return py_cio_io(CIO_IO_WRITE, args, "is*n:write");
}

static PyObject *py_cio_connect(PyObject *self, PyObject *args)
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

static PyObject *py_cio_accept(PyObject *self, PyObject *args)
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

static PyObject *py_cio_recv(PyObject *self, PyObject *args)
{
	return py_cio_io(CIO_IO_RECV, args, "iw*ni:recv");
}

static PyObject *py_cio_send(PyObject *self, PyObject *args)
{
	return py_cio_io(CIO_IO_SEND, args, "is*ni:send");
}

static PyMethodDef py_cio_methods[] = {
	{ "launch", py_cio_launch, METH_VARARGS, NULL },

	{ "read", py_cio_read, METH_VARARGS, NULL },
	{ "write", py_cio_write, METH_VARARGS, NULL },

	{ "connect", py_cio_connect, METH_VARARGS, NULL },
	{ "accept", py_cio_accept, METH_VARARGS, NULL },
	{ "recv", py_cio_recv, METH_VARARGS, NULL },
	{ "send", py_cio_send, METH_VARARGS, NULL },

	{ NULL, NULL, 0, NULL }
};

static struct PyModuleDef py_cio_module = {
	PyModuleDef_HEAD_INIT,
	"_cio",
	NULL,
	-1,
	py_cio_methods,
};

PyMODINIT_FUNC PyInit__cio(void)
{
	return PyModule_Create(&py_cio_module);
}

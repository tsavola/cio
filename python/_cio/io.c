/*
 * Copyright (c) 2010  Timo Savola
 */

#include <Python.h>

#include "io.h"
#include "socket.h"

#include <stddef.h>

#include <sys/types.h>

#include "cio/io-internal.h"
#include "cio/io.h"
#include "cio/socket.h"
#include "cio/trace.h"

#include "thread.h"

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

PyObject *py_cio_read(PyObject *self, PyObject *args)
{
	cio_trace(__func__);

	return py_cio_io(CIO_IO_READ, args, "iw*n:read");
}

PyObject *py_cio_write(PyObject *self, PyObject *args)
{
	cio_trace(__func__);

	return py_cio_io(CIO_IO_WRITE, args, "is*n:write");
}

PyObject *py_cio_recv(PyObject *self, PyObject *args)
{
	cio_trace(__func__);

	return py_cio_io(CIO_IO_RECV, args, "iw*ni:recv");
}

PyObject *py_cio_send(PyObject *self, PyObject *args)
{
	cio_trace(__func__);

	return py_cio_io(CIO_IO_SEND, args, "is*ni:send");
}

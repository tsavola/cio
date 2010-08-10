/*
 * Copyright (c) 2010  Timo Savola
 */

#include <Python.h>

#include "file.h"

#include <stddef.h>

#include "cio/file.h"

PyObject *py_cio_close(PyObject *self, PyObject *args)
{
	int fd;
	int ret;
	int err;

	if (!PyArg_ParseTuple(args, "i:close", &fd))
		return NULL;

	ret = cio_close(fd);
	err = errno;

	if (ret < 0 && err == EINTR)
		PyErr_SetInterrupt();

	if (ret < 0) {
		errno = err;
		return PyErr_SetFromErrno(PyExc_IOError);
	}

	return PyLong_FromLong(ret);
}

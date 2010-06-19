/*
 * Copyright (c) 2010  Timo Savola
 */

#include <Python.h>

#include "routine.h"

#include <stddef.h>

#include "cio/routine.h"

#include "thread.h"

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

PyObject *py_cio_launch(PyObject *self, PyObject *args)
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

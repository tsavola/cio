/*
 * Copyright (c) 2010  Timo Savola
 */

#include <Python.h>

#include "routine.h"

#include <stddef.h>

#include "cio/routine.h"
#include "cio/trace.h"

#include "thread.h"

static void py_cio_routine_main(void *arg)
{
	cio_tracef("%s: enter", __func__);

	PyObject *callable = *(PyObject **) arg;
	PyObject *result;

	py_cio_thread_restore();

	result = PyObject_CallObject(callable, NULL);
	if (result == NULL)
		PyErr_Print();

	Py_XDECREF(result);
	Py_DECREF(callable);

	py_cio_thread_save();

	cio_tracef("%s: leave", __func__);
}

PyObject *py_cio_routine(PyObject *self, PyObject *args)
{
	cio_trace(__func__);

	PyObject *callable;
	int ret;

	if (!PyArg_ParseTuple(args, "O:routine", &callable))
		return NULL;

	if (!PyCallable_Check(callable)) {
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return NULL;
	}

	Py_INCREF(callable);

	py_cio_thread_save();
	ret = cio_routine(py_cio_routine_main, &callable, sizeof (PyObject *));
	py_cio_thread_restore();

	if (ret < 0) {
		Py_DECREF(callable);
		return PyErr_NoMemory();
	}

	Py_INCREF(Py_None);
	return Py_None;
}

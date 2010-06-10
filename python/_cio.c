/*
 * Copyright (c) 2010  Timo Savola
 */

#include <stddef.h>

#include <Python.h>

#include <cio/routine.h>

static void py_cio_routine(void *arg)
{
	PyObject *callable = *(PyObject **) arg;
	PyObject *result;

	result = PyObject_CallObject(callable, NULL);
	if (result == NULL)
		PyErr_Print();

	Py_XDECREF(result);
	Py_DECREF(callable);
}

static PyObject *py_cio_launch(PyObject *self, PyObject *args)
{
	PyObject *callable;

	if (!PyArg_ParseTuple(args, "O:launch", &callable))
		return NULL;

	if (!PyCallable_Check(callable)) {
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return NULL;
	}

	Py_INCREF(callable);

	if (cio_launch(py_cio_routine, &callable, sizeof (PyObject *)) < 0) {
		Py_DECREF(callable);

		PyErr_SetString(PyExc_MemoryError, "routine launch failed");
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef py_cio_methods[] = {
	{ "launch", py_cio_launch, METH_VARARGS, NULL },
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

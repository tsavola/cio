/*
 * Copyright (c) 2010  Timo Savola
 */

#include <Python.h>

#include <stddef.h>

#include "io.h"
#include "routine.h"
#include "socket.h"

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

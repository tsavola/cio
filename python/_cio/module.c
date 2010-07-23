/*
 * Copyright (c) 2010  Timo Savola
 */

#include <Python.h>

#include <stddef.h>

#include "channel.h"
#include "io.h"
#include "routine.h"
#include "socket.h"

static PyMethodDef py_cio_methods[] = {
	{ "routine", py_cio_routine, METH_VARARGS, NULL },

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
	PyObject *module;
	PyObject *channel;

	module = PyModule_Create(&py_cio_module);
	if (module == NULL)
		goto fail;

	channel = py_cio_channel_type();
	if (channel == NULL)
		goto fail;

	if (PyModule_AddObject(module, "channel", channel) < 0)
		goto fail;

	return module;

fail:
	Py_XDECREF(module);
	return NULL;
}

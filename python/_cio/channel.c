/*
 * Copyright (c) 2010  Timo Savola
 */

#include <Python.h>

#include "channel.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>

#include "cio/channel.h"
#include "cio/trace.h"

#include "thread.h"

struct py_cio_channel {
	PyObject_HEAD
	struct cio_channel *channel;
};

static struct cio_channel *py_cio_channel(PyObject *object)
{
	return ((struct py_cio_channel *) object)->channel;
}

static PyObject *py_cio_channel_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	cio_trace(__func__);

	struct cio_channel *c = cio_channel_create(sizeof (PyObject *));
	if (c == NULL)
		return PyErr_NoMemory();

	PyObject *self = type->tp_alloc(type, 0);
	if (self)
		((struct py_cio_channel *) self)->channel = c;
	else
		cio_channel_unref(c);

	return self;
}

static void py_cio_channel_dealloc(PyObject *self)
{
	cio_trace(__func__);

	cio_channel_unref(py_cio_channel(self));
	Py_TYPE(self)->tp_free(self);
}

static PyObject *py_cio_channel_close(PyObject *self, PyObject *args)
{
	cio_trace(__func__);

	cio_channel_close(py_cio_channel(self));

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *py_cio_channel_read(PyObject *self, PyObject *args)
{
	cio_trace(__func__);

	PyObject *item;
	PyObject *tuple;
	int ret;
	int err;

	py_cio_thread_save();

	ret = cio_channel_read(py_cio_channel(self), &item, sizeof (PyObject *));
	err = errno;

	if (ret < 0 && errno == EINTR)
		PyErr_SetInterrupt();

	py_cio_thread_restore();

	if (ret < 0) {
		errno = err;
		return PyErr_SetFromErrno(PyExc_IOError);
	}

	if (ret == 0) {
		cio_tracef("%s: closed", __func__);

		Py_INCREF(Py_None);
		item = Py_None;
	} else {
		cio_tracef("%s: item %p", __func__, item);
	}

	tuple = PyTuple_Pack(2, item, PyBool_FromLong(ret));
	if (tuple == NULL)
		Py_DECREF(item);

	return tuple;
}

static PyObject *py_cio_channel_write(PyObject *self, PyObject *args)
{
	cio_trace(__func__);

	PyObject *item;
	int ret;
	int err;

	if (!PyArg_ParseTuple(args, "O:write", &item))
		return NULL;

	cio_tracef("%s: item %p", __func__, item);

	Py_INCREF(item);

	py_cio_thread_save();

	ret = cio_channel_write(py_cio_channel(self), &item, sizeof (PyObject *));
	err = errno;

	if (ret < 0 && errno == EINTR)
		PyErr_SetInterrupt();

	py_cio_thread_restore();

	if (ret != 1)
		Py_DECREF(item);

	if (ret < 0) {
		errno = err;
		return PyErr_SetFromErrno(PyExc_IOError);
	}

	if (ret == 0)
		cio_tracef("%s: closed", __func__);

	return PyBool_FromLong(ret);
}

static PyMethodDef py_cio_channel_methods[] = {
	{ "close", py_cio_channel_close, METH_NOARGS, NULL },
	{ "read", py_cio_channel_read, METH_NOARGS, NULL },
	{ "write", py_cio_channel_write, METH_VARARGS, NULL },
	{ NULL }
};

static PyTypeObject py_cio_channel_type_object = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"_cio.channel",                 /* tp_name */
	sizeof (struct py_cio_channel), /* tp_basicsize */
	0,                              /* tp_itemsize */
	py_cio_channel_dealloc,         /* tp_dealloc */
	NULL,                           /* tp_print */
	NULL,                           /* tp_getattr */
	NULL,                           /* tp_setattr */
	NULL,                           /* tp_reserved */
	NULL,                           /* tp_repr */
	NULL,                           /* tp_as_number */
	NULL,                           /* tp_as_sequence */
	NULL,                           /* tp_as_mapping */
	NULL,                           /* tp_hash  */
	NULL,                           /* tp_call */
	NULL,                           /* tp_str */
	NULL,                           /* tp_getattro */
	NULL,                           /* tp_setattro */
	NULL,                           /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,             /* tp_flags */
	NULL,                           /* tp_doc */
	NULL,                           /* tp_traverse */
	NULL,                           /* tp_clear */
	NULL,                           /* tp_richcompare */
	0,                              /* tp_weaklistoffset */
	NULL,                           /* tp_iter */
	NULL,                           /* tp_iternext */
	py_cio_channel_methods,         /* tp_methods */
	NULL,                           /* tp_members */
	NULL,                           /* tp_getset */
	NULL,                           /* tp_base */
	NULL,                           /* tp_dict */
	NULL,                           /* tp_descr_get */
	NULL,                           /* tp_descr_set */
	0,                              /* tp_dictoffset */
	NULL,                           /* tp_init */
	NULL,                           /* tp_alloc */
	py_cio_channel_new,             /* tp_new */
};

PyObject *py_cio_channel_type(void)
{
	cio_trace(__func__);

	if (PyType_Ready(&py_cio_channel_type_object) < 0)
		return NULL;

	Py_INCREF(&py_cio_channel_type_object);
	return (PyObject *) &py_cio_channel_type_object;
}

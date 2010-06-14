/*
 * Copyright (c) 2010  Timo Savola
 */

#include <stddef.h>

#include <Python.h>

#include <cio/io.h>
#include <cio/routine.h>

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

static PyObject *py_cio_io(PyObject *args, const char *format, ssize_t (*func)(int, void *, size_t))
{
	int fd;
	Py_buffer buf;
	Py_ssize_t size;
	ssize_t len;
	int err;

	if (!PyArg_ParseTuple(args, format, &fd, &buf, &size))
		return NULL;

	py_cio_thread_save();

	if (size > buf.len)
		size = buf.len;

	len = func(fd, buf.buf, size);
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
	return py_cio_io(args, "iw*n:read", cio_read);
}

static PyObject *py_cio_write(PyObject *self, PyObject *args)
{
	return py_cio_io(args, "is*n:write", (ssize_t (*)(int, void *, size_t)) cio_write);
}

static PyMethodDef py_cio_methods[] = {
	{ "launch", py_cio_launch, METH_VARARGS, NULL },
	{ "read", py_cio_read, METH_VARARGS, NULL },
	{ "write", py_cio_write, METH_VARARGS, NULL },
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

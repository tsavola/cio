/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef PY_CIO_SOCKET_H
#define PY_CIO_SOCKET_H

#include <Python.h>

#include "cio/attr.h"

PyObject *py_cio_connect(PyObject *, PyObject *) CIO_INTERNAL;
PyObject *py_cio_accept(PyObject *, PyObject *) CIO_INTERNAL;
PyObject *py_cio_recv(PyObject *, PyObject *) CIO_INTERNAL;
PyObject *py_cio_send(PyObject *, PyObject *) CIO_INTERNAL;

#endif

/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef PY_CIO_IO_H
#define PY_CIO_IO_H

#include <Python.h>

#include "cio/attr.h"

PyObject *py_cio_read(PyObject *, PyObject *) CIO_INTERNAL;
PyObject *py_cio_write(PyObject *, PyObject *) CIO_INTERNAL;

#endif

/*
 * Copyright (c) 2010  Timo Savola
 */

#include <Python.h>

#include "thread.h"

static __thread PyThreadState *py_cio_thread_state;

void py_cio_thread_save(void)
{
	py_cio_thread_state = PyEval_SaveThread();
}

void py_cio_thread_restore(void)
{
	PyEval_RestoreThread(py_cio_thread_state);
}

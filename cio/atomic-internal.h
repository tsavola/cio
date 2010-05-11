/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_ATOMIC_INTERNAL_H
#define CIO_ATOMIC_INTERNAL_H

static inline void cio_increment(int *value)
{
	__sync_add_and_fetch(value, 1);
}

static inline int cio_decrement(int *value)
{
	return __sync_add_and_fetch(value, -1);
}

#endif

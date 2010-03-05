/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_IO_H
#define CIO_IO_H

/**
 * @defgroup io I/O
 * @code #include <cio/io.h> @endcode
 * @{
 */

#include <stddef.h>

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

ssize_t cio_read(int fd, void *buf, size_t bufsize);
ssize_t cio_read_full(int fd, void *buf, size_t bufsize);

ssize_t cio_write(int fd, const void *buf, size_t bufsize);

ssize_t cio_sendfile(int out_fd, int in_fd, off_t *offset, size_t count);

#ifdef __cplusplus
}
#endif

/** @} */

#endif

/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_SOCKET_H
#define CIO_SOCKET_H

/**
 * @defgroup socket Socket
 * @code #include <cio/socket.h> @endcode
 * @{
 */

#include <stddef.h>

#include <sys/types.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

int cio_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int cio_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int cio_accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);

ssize_t cio_recv(int sockfd, void *buf, size_t len, int flags);
ssize_t cio_send(int sockfd, const void *buf, size_t len, int flags);

#ifdef __cplusplus
}
#endif

/** @} */

#endif

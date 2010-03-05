#include <stdbool.h>
#include <stdio.h>

#include <netdb.h>
#include <unistd.h>

#include <cio/channel.h>
#include <cio/routine.h>
#include <cio/socket.h>
#include <cio/time.h>

#include "assert.h"

#define NODE    "localhost"
#define SERVICE "26473"

struct args {
	struct addrinfo *info;
	struct cio_channel *exit;
};

static void finish(struct args *args)
{
	assert(cio_channel_write(args->exit, NULL, 0) == 1);

	cio_channel_unref(args->exit);
}

struct connect_args {
	struct args common;
};

static void connector(void *connect_args)
{
	struct connect_args *args = connect_args;
	struct addrinfo *info = args->common.info;

	int fd = socket(info->ai_family, info->ai_socktype | SOCK_NONBLOCK, info->ai_protocol);
	assert(fd >= 0);

	printf("connect\n");

	assert(cio_connect(fd, info->ai_addr, info->ai_addrlen) == 0);

	printf("connected\n");

	close(fd);

	finish(&args->common);
}

struct accept_args {
	struct args common;
};

static void acceptor(void *accept_args)
{
	struct accept_args *args = accept_args;
	struct addrinfo *info = args->common.info;

	int fd = socket(info->ai_family, info->ai_socktype | SOCK_NONBLOCK, info->ai_protocol);
	assert(fd >= 0);

	int opt = 1;
	assert(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt)) == 0);

	assert(bind(fd, info->ai_addr, info->ai_addrlen) == 0);
	assert(listen(fd, SOMAXCONN) == 0);

	printf("accept\n");

	int connfd = cio_accept(fd, NULL, NULL, SOCK_NONBLOCK);
	assert(connfd >= 0);

	printf("accepted\n");

	struct timespec interval = {
		.tv_sec = 1,
	};
	// assert(cio_sleep(&interval) == 0);

	close(connfd);
	close(fd);

	finish(&args->common);
}

void test_socket(void)
{
	struct cio_channel *exit = cio_channel_create(0);
	assert(exit);

	struct addrinfo *info;
	struct addrinfo hints = {
		.ai_flags = AI_ADDRCONFIG,
		.ai_socktype = SOCK_STREAM,
	};

	assert(getaddrinfo(NODE, SERVICE, &hints, &info) == 0);

	struct accept_args aargs = {
		.common.info = info,
		.common.exit = cio_channel_ref(exit),
	};

	assert(cio_launch(acceptor, &aargs, sizeof (aargs)) == 0);

	struct connect_args cargs = {
		.common.info = info,
		.common.exit = cio_channel_ref(exit),
	};

	assert(cio_launch(connector, &cargs, sizeof (cargs)) == 0);

	assert(cio_channel_read(exit, NULL, 0) == 1);
	assert(cio_channel_read(exit, NULL, 0) == 1);
	cio_channel_unref(exit);

	freeaddrinfo(info);
}

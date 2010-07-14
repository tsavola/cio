/*
 * Copyright (c) 2010  Timo Savola
 */

#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <sys/signalfd.h>

#include <cio/io.h>
#include <cio/routine.hpp>
#include <cio/socket.h>
#include <cio/util.h>

#define FAVICON "/usr/share/pixmaps/diffuse.png"
#define VERBOSE false

static void handler(int &fd)
{
	if (VERBOSE)
		printf("[%d] connected\n", fd);

	while (true) {
		char request[4096];
		ssize_t len = 0;

		while (true) {
			int ret = cio_read(fd, request + len, sizeof (request) - len);
			if (ret < 0) {
				if (errno != ECONNRESET)
					perror("read");
				goto fail;
			}

			if (ret == 0)
				goto fail;

			len += ret;

			bool newline = true;
			for (int i = 0; i < len; i++) {
				if (request[i] == '\n') {
					if (newline) {
						if (i > 0 && request[i - 1] == '\r')
							len = i - 1;
						else
							len = i;
						goto got_request;
					}
					newline = true;
				} else if (request[i] != '\r') {
					newline = false;
				}
			}

			if (len == sizeof (request))
				goto fail;
		}

	got_request:
		for (int i = 0; i < len; i++)
			if (request[i] == '\r' || request[i] == '\n') {
				request[i] = '\0';
				break;
			}

		if (strstr(request, "GET ") != request)
			goto fail;

		char *path_end = strstr(request, " HTTP/1.");
		if (path_end == NULL)
			goto fail;
		*path_end = '\0';

		const char *request_path = request + 4;

		if (VERBOSE)
			printf("[%d] %s\n", fd, request_path);

		if (strcmp(request_path, "/") == 0) {
			const char response[] =
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: 90\r\n"
				"Connection: keep-alive\r\n"
				"\r\n"
				"<html><head><title>hello world</title></head><body><img src=\"favicon.ico\"/></body></html>\n";

			if (cio_write(fd, response, sizeof (response)) < 0) {
				perror("write");
				goto fail;
			}

		} else if (strcmp(request_path, "/favicon.ico") == 0) {
			int file_fd = open(FAVICON, O_RDONLY);
			if (file_fd < 0) {
				perror("open");
				goto fail;
			}

			struct stat st;
			if (fstat(file_fd, &st) < 0) {
				perror("stat");
				close(file_fd);
				goto fail;
			}

			char headers[256];
			snprintf(headers, sizeof (headers),
			         "HTTP/1.1 200 OK\r\n"
			         "Content-Type: image/png\r\n"
			         "Content-Length: %lu\r\n"
			         "Connection: keep-alive\r\n"
			         "\r\n",
			         st.st_size);

			if (cio_write(fd, headers, strlen(headers)) < 0) {
				perror("write");
				close(file_fd);
				goto fail;
			}

			if (cio_sendfile(fd, file_fd, NULL, st.st_size) < 0) {
				perror("sendfile");
				close(file_fd);
				goto fail;
			}

			close(file_fd);

		} else {
			const char response[] =
				"HTTP/1.1 404 Not Found\r\n"
				"Content-Length: 0\r\n"
				"Connection: keep-alive\r\n"
				"\r\n";

			if (cio_write(fd, response, sizeof (response)) < 0) {
				perror("write");
				goto fail;
			}
		}
	}

fail:
	close(fd);

	if (VERBOSE)
		printf("[%d] disconnected\n", fd);
}

static void listener(int &listen_fd)
{
	while (true) {
		int conn_fd = cio_accept4(listen_fd, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);

		if (conn_fd < 0) {
			if (errno == ECONNABORTED)
				continue;

			perror("accept");
			break;
		}

		cio::routine(handler, conn_fd);
	}

	close(listen_fd);
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		fprintf(stderr, "Usage: %s PORT ADDRESS [ADDRESS ...]\n", argv[0]);
		return 1;
	}

	sigset_t sigs;
	sigfillset(&sigs);
	sigdelset(&sigs, SIGILL);
	sigdelset(&sigs, SIGABRT);
	sigdelset(&sigs, SIGFPE);
	sigdelset(&sigs, SIGSEGV);
	sigdelset(&sigs, SIGBUS);

	if (sigprocmask(SIG_SETMASK, &sigs, NULL) < 0) {
		perror("sigprocmask");
		return 1;
	}

	sigemptyset(&sigs);
	sigaddset(&sigs, SIGINT);
	sigaddset(&sigs, SIGTERM);

	int signal_fd = signalfd(-1, &sigs, SFD_NONBLOCK | SFD_CLOEXEC);
	if (signal_fd < 0) {
		perror("signalfd");
		return 1;
	}

	int listeners = 0;
	const char *service = argv[1];

	for (int argn = 2; argn < argc; argn++) {
		const char *node = argv[argn];

		struct addrinfo *info;
		struct addrinfo hints = { 0 };

		hints.ai_flags = AI_ADDRCONFIG;
		hints.ai_socktype = SOCK_STREAM;

		int ret = getaddrinfo(node, service, &hints, &info);
		if (ret < 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
			continue;
		}

		for (struct addrinfo *i = info; i; i = i->ai_next) {
			int fd = socket(i->ai_family, i->ai_socktype | SOCK_NONBLOCK | SOCK_CLOEXEC, i->ai_protocol);
			if (fd < 0)
				continue;

			int value = 1;
			if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof (value)) < 0)
				perror("setsockopt(SO_REUSEADDR)");

			if (bind(fd, i->ai_addr, i->ai_addrlen) < 0) {
				perror("bind");
				goto fail;
			}

			if (listen(fd, SOMAXCONN) < 0) {
				perror("listen");
				goto fail;
			}

			cio::routine(listener, fd);

			listeners++;
			continue;

		fail:
			close(fd);
		}

		freeaddrinfo(info);
	}

	if (listeners > 0) {
		struct signalfd_siginfo info;

		while (cio_read(signal_fd, &info, sizeof (info)) == sizeof (info))
			switch (info.ssi_signo) {
			case SIGINT:
			case SIGTERM:
				printf("Terminated\n");
				return 0;
			}
	}

	return 1;
}

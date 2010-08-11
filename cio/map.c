/*
 * Copyright (c) 2010  Timo Savola
 */

#include "map-internal.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

#include <unistd.h>

static int cio_map_alloc(struct cio_map *map)
{
	if (map->vector == NULL) {
		map->length = sysconf(_SC_OPEN_MAX);
		if (map->length < 0)
			return -1;

		map->vector = calloc(map->length, sizeof (void *));
		if (map->vector == NULL)
			return -1;
	}

	return 0;
}

static int cio_map_check(const struct cio_map *map, int fd)
{
	if (map->vector == NULL) {
		errno = ENOENT;
		return -1;
	}

	if (fd < 0 || fd >= map->length) {
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int cio_map_add(struct cio_map *map, int fd, void *node)
{
	if (cio_map_alloc(map) < 0)
		return -1;

	if (cio_map_check(map, fd) < 0)
		return -1;

	if (map->vector[fd]) {
		errno = EEXIST;
		return -1;
	}

	map->vector[fd] = node;
	return 0;
}

void *cio_map_find(const struct cio_map *map, int fd)
{
	if (cio_map_check(map, fd) < 0)
		return NULL;

	return map->vector[fd];
}

int cio_map_remove(struct cio_map *map, int fd)
{
	if (cio_map_check(map, fd) < 0)
		return -1;

	if (map->vector[fd] == NULL) {
		errno = ENOENT;
		return -1;
	}

	map->vector[fd] = NULL;
	return 0;
}

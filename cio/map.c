/*
 * Copyright (c) 2010  Timo Savola
 */

#include "map-internal.h"

#include <stddef.h>

#include <unistd.h>

static int cio_map_impl_array = -1;

static int cio_map_init(void)
{
	if (cio_map_impl_array >= 0)
		return 0;

	long length = sysconf(_SC_OPEN_MAX);
	if (length < 0)
		return -1;

	cio_map_impl_array = (length <= CIO_ARRAY_LIMIT);
	return 0;
}

int cio_map_add(struct cio_map *map, int fd, void *node)
{
	if (cio_map_init() < 0)
		return -1;

	if (cio_map_impl_array)
		return cio_map_array_add(&map->u.array, fd, node);
	else
		return cio_map_tree_add(&map->u.tree, fd, node);
}

void *cio_map_find(const struct cio_map *map, int fd)
{
	if (cio_map_init() < 0)
		return NULL;

	if (cio_map_impl_array)
		return cio_map_array_find(&map->u.array, fd);
	else
		return cio_map_tree_find(&map->u.tree, fd);
}

int cio_map_remove(struct cio_map *map, int fd)
{
	if (cio_map_init() < 0)
		return -1;

	if (cio_map_impl_array)
		return cio_map_array_remove(&map->u.array, fd);
	else
		return cio_map_tree_remove(&map->u.tree, fd);
}

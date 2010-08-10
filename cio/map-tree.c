/*
 * Copyright (c) 2010  Timo Savola
 */

#include "map-internal.h"

#include <errno.h>
#include <stddef.h>

#include <search.h>

static int cio_map_compare(const void *void_left, const void *void_right)
{
	const struct cio_map_node *left = void_left;
	const struct cio_map_node *right = void_right;

	if (left->fd < right->fd)
		return -1;

	if (left->fd > right->fd)
		return 1;

	return 0;
}

int cio_map_add(struct cio_map *map, int fd, void *void_node)
{
	struct cio_map_node *node = void_node;
	struct cio_map_node **ret;

	node->fd = fd;

	ret = tsearch(node, &map->root, cio_map_compare);
	if (ret == NULL)
		return -1;

	if (*ret != node) {
		errno = EEXIST;
		return -1;
	}

	return 0;
}

void *cio_map_find(const struct cio_map *map, int fd)
{
	struct cio_map_node key = {
		fd,
	};

	return tfind(&key, &map->root, cio_map_compare);
}

int cio_map_remove(struct cio_map *map, int fd)
{
	struct cio_map_node key = {
		fd,
	};

	if (tdelete(&key, &map->root, cio_map_compare) == NULL) {
		errno = ENOENT;
		return -1;
	}

	return 0;
}

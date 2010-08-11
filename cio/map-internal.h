/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_MAP_INTERNAL_H
#define CIO_MAP_INTERNAL_H

#include "map-tree-internal.h"
#include "map-array-internal.h"

/**
 * File descriptor-addressed intrusive map structure.
 */
struct cio_map {
	union {
		struct cio_map_array array;
		struct cio_map_tree tree;
	} u;
};

/**
 * The node structs in cio_map must embed this at the start.
 */
struct cio_map_node {
	union {
		struct cio_map_array_node array;
		struct cio_map_tree_node tree;
	} u;
};

/**
 * TODO
 */
int cio_map_add(struct cio_map *map, int fd, void *node);

/**
 * TODO
 */
void *cio_map_find(const struct cio_map *map, int fd);

/**
 * TODO
 */
int cio_map_remove(struct cio_map *map, int fd);

#endif

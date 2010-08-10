/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_MAP_INTERNAL_H
#define CIO_MAP_INTERNAL_H

/**
 * File descriptor-addressed intrusive map structure.
 */
struct cio_map;

/**
 * The node structs in cio_map must embed this at the start.
 */
struct cio_map_node;

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

#ifdef CIO_MAP_TREE
# include "map-tree-internal.h"
#endif

#ifdef CIO_MAP_ARRAY
# include "map-array-internal.h"
#endif

#endif

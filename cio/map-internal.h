/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_MAP_INTERNAL_H
#define CIO_MAP_INTERNAL_H

/**
 * File descriptor-addressed map.
 */
struct cio_map {
	int length;
	void **vector;
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

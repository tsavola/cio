/*
 * Copyright (c) 2010  Timo Savola
 */

#define CIO_ARRAY_LIMIT   4096

struct cio_map_array {
	int length;
	void **vector;
};

struct cio_map_array_node {};

int cio_map_array_add(struct cio_map_array *map, int fd, void *node);
void *cio_map_array_find(const struct cio_map_array *map, int fd);
int cio_map_array_remove(struct cio_map_array *map, int fd);

/*
 * Copyright (c) 2010  Timo Savola
 */

struct cio_map_tree {
	void *root;
};

struct cio_map_tree_node {
	int fd;
};

int cio_map_tree_add(struct cio_map_tree *map, int fd, void *node);
void *cio_map_tree_find(const struct cio_map_tree *map, int fd);
int cio_map_tree_remove(struct cio_map_tree *map, int fd);

/*
 * Copyright (c) 2010  Timo Savola
 */

#ifndef CIO_LIST_INTERNAL_H
#define CIO_LIST_INTERNAL_H

#include <stddef.h>

/**
 * Generic intrusive singly-linked list.  The node type must be a struct
 * with a "next" pointer member.
 */
struct cio_list {
	void *head;
	void *tail;
};

/**
 * Get the node at the front of the list.
 *
 * @param type  node type
 * @param list  struct cio_list pointer
 * @return node pointer or NULL if the list is empty
 */
#define cio_list_head(type, list) \
	((type *) (list)->head)

/**
 * Link a node at the back of the list.
 *
 * @param type  node type
 * @param list  struct cio_list pointer
 * @param node  node pointer
 */
#define cio_list_append(type, list, node) \
	do { \
		if ((list)->tail) \
			((type *) (list)->tail)->next = (node); \
		else \
			(list)->head = (node); \
		(list)->tail = (node); \
		((type *) (node))->next = NULL; \
	} while (0)

/**
 * Unlink the node at the front of the list.  The list must not be empty.
 *
 * @param type  node type
 * @param list  struct cio_list pointer
 */
#define cio_list_remove_head(type, list) \
	do { \
		(list)->head = ((type *) (list)->head)->next; \
		if ((list)->head == NULL) \
			(list)->tail = NULL; \
	} while (0)

#endif

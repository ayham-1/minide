#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <stdint.h>
#include <unistd.h>

typedef struct {
	void * root; // avl_node_t

	void * nodes;
	void * count;
} avl_tree_t;

void avl_tree_append(avl_node_t * root, size_t key, size_t data);

typedef struct {
	struct avl_node_t * left;
	struct avl_node_t * right;

	size_t height;
	size_t bf;

	size_t key;
	size_t data;
} avl_node_t;

#endif

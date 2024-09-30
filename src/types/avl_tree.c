#include "minide/types/avl_tree.h"

#include <assert.h>
#include <malloc.h>
#include <string.h>

#define NODES_INITIAL_CAPACITY 3
#define NODES_COUNT_MULT 2

void avl_tree_create(avl_tree_t * root)
{
	assert(root);

	root->root = NULL;
	root->nodes = malloc(sizeof(avl_node_t) * NODES_INITIAL_CAPACITY);
	root->count = 0;
}

void avl_tree_add(avl_tree_t * root, size_t key, size_t data) { assert(root); }

void avl_tree_remove(avl_tree_t * root, size_t key);

avl_node_t * avl_tree_find(avl_tree_t * root, size_t key);

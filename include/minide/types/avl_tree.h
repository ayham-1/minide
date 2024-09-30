#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <stdbool.h>
#include <unistd.h>

typedef bool (*avl_cmp_function)(size_t d1, size_t d2);
typedef struct avl_node_t avl_node_t;

typedef struct {
	avl_node_t * root;

	avl_node_t * nodes;
	size_t count;
} avl_tree_t;

void avl_tree_create(avl_tree_t * root);
void avl_tree_cleanup(avl_tree_t * root);

void avl_tree_add(avl_tree_t * root, size_t key, size_t data);
void avl_tree_remove(avl_tree_t * root, size_t key);
avl_node_t * avl_tree_find(avl_tree_t * root, size_t key);

void avl_tree_rebalance(avl_tree_t * root, avl_node_t * node);
void __avl_tree_left_rot(avl_tree_t * root, avl_node_t * node);

struct avl_node_t {
	struct avl_node_t * left;
	struct avl_node_t * right;

	size_t height;
	size_t bf;

	size_t key;
	size_t data;
};

#endif

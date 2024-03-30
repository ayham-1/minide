#include "minide/buffers/node.h"

#include <assert.h>

#define NODES_REALLOC_FACTOR 1.5
#define NODES_INITIAL_CAPACITY 3

void buffer_lnode_init(buffer_lnode * lnode)
{
	assert(lnode);
	lnode->next = NULL;
	lnode->prev = NULL;
	lnode->count = 0;
	lnode->capacity = NODES_INITIAL_CAPACITY;

	lnode->nodes = malloc(sizeof(buffer_node) * lnode->capacity);
}

void buffer_lnode_clean(buffer_lnode * lnode) { free(lnode->nodes); }

void buffer_lnode_add_config(buffer_lnode * lnode, text_render_config config, size_t renderer)
{
	if (lnode->count + 1 >= lnode->capacity) {
		lnode->capacity *= NODES_REALLOC_FACTOR;
		lnode->nodes = realloc(lnode->nodes, sizeof(buffer_node) * lnode->capacity);
		// TODO(ayham-1): you have to reassign prev & next
	}

	buffer_node * current = &lnode->nodes[lnode->count];

	if (lnode->count == 0) {
		current->prev = NULL;
	}

	if (lnode->count + 1 >= 2) {
		lnode->nodes[lnode->count].next = (struct buffer_node *)current;
		current->prev = (struct buffer_node *)&lnode->nodes[lnode->count];
	}

	current->next = NULL;
	current->config = config;

	lnode->count++;
}

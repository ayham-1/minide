#include "minide/ui/buffers/node.h"

#include <assert.h>

#define NODES_REALLOC_FACTOR 1.5
#define NODES_INITIAL_CAPACITY 3

void buffer_lnode_init(buffer_lnode * lnode)
{
	assert(lnode);
	lnode->lines_index_next = 0;
	lnode->lines_index_prev = 0;
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
	}

	buffer_node * current = &lnode->nodes[lnode->count];

	if (lnode->count == 0) {
		current->node_index_prev = 0;
	}

	if (lnode->count + 1 >= 2) {
		lnode->nodes[lnode->count - 1].node_index_next = lnode->count;
		current->node_index_prev = lnode->count;
	}

	current->node_index_next = 0;
	current->config = config;

	// always keep last node index in the first node
	lnode->nodes[0].node_index_prev = lnode->count;
	// always keep node number in the current node
	current->node_number = lnode->count + 1;

	lnode->count++;
}

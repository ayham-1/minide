#ifndef BUFFER_NODE_H
#define BUFFER_NODE_H

#include "minide/text_renderer.h"

typedef struct {
	size_t node_index_prev;
	size_t node_index_next;
	size_t node_number;

	size_t renderer_index;
	text_render_config config;
} buffer_node;

typedef struct {
	size_t lines_index_prev;
	size_t lines_index_next;
	size_t line_number;

	buffer_node * nodes;
	size_t count;
	size_t capacity;
} buffer_lnode;

void buffer_lnode_init(buffer_lnode *);
void buffer_lnode_clean(buffer_lnode *);

void buffer_lnode_add_config(buffer_lnode *, text_render_config, size_t);

#endif

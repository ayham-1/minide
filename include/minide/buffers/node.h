#ifndef BUFFER_NODE_H
#define BUFFER_NODE_H

#include "minide/text_renderer.h"

typedef struct {
	struct buffer_node * prev;
	struct buffer_node * next;

	size_t renderer_index;
	text_render_config config;
} buffer_node;

typedef struct {
	struct buffer_lnode * prev;
	struct buffer_lnode * next;

	buffer_node * nodes;
	size_t count;
	size_t capacity;
} buffer_lnode;

void buffer_lnode_init(buffer_lnode *);
void buffer_lnode_clean(buffer_lnode *);

void buffer_lnode_add_config(buffer_lnode *, text_render_config, size_t);

#endif

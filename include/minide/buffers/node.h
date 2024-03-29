#ifndef BUFFER_NODE_H
#define BUFFER_NODE_H

#include "minide/text_renderer.h"

typedef struct {
	struct buf_node * prev;
	struct buf_node * next;

	size_t renderer_index;
	text_render_config config;
} buf_node;

typedef struct {
	struct buf_line_node * prev;
	struct buf_line_node * next;

	buf_node * nodes;
	size_t nodes_cnt;
} buf_line_node;

#endif

#include "minide/buffers/meta.h"

#include <assert.h>

#define LINES_REALLOC_FACTOR 1.5
#define LINES_INITIAL_CAPACITY 3

void buffer_init(buffer_view * view)
{
	assert(view);
	assert(view->renderer);

	view->nlines = 0;
	view->lnodes_capacity = LINES_INITIAL_CAPACITY;
	view->lnodes = malloc(sizeof(buffer_lnode) * view->lnodes_capacity);
};

void buffer_clean(buffer_view * view)
{
	for (size_t i = 0; i < view->nlines; i++) {
	}
	free(view->lnodes);
}

buffer_lnode * buffer_append_line(buffer_view * view, text_render_config config)
{
	if (view->nlines + 1 >= view->lnodes_capacity) {
		view->lnodes_capacity *= LINES_REALLOC_FACTOR;
		view->lnodes = realloc(view->lnodes, sizeof(buffer_lnode) * view->lnodes_capacity);
		// TODO(ayham-1): you have to reassign prev & next
	}

	buffer_lnode * current = &view->lnodes[view->nlines];

	buffer_lnode_init(current);
	buffer_lnode_add_config(current, config, 0);

	if (view->nlines == 0) {
		current->prev = NULL;
	}

	if (view->nlines >= 1) {
		view->lnodes[view->nlines].next = (struct buffer_lnode *)current;
		current->prev = (struct buffer_lnode *)&view->lnodes[view->nlines];
	}

	current->next = NULL;
	view->nlines++;

	return current;
}

void buffer_render_all(buffer_view * view)
{
	GLfloat current_x = view->scr_x;
	GLfloat current_y = view->scr_y;

	// TODO(ayham-1): configs need to be automatically all set to the same max_line_width_chars
	// this is a setting of the buffer, and not individual lines

	if (view->nlines && view->lnodes[0].count) {
		view->lnodes[0].nodes[0].config.origin_x = current_x;
		view->lnodes[0].nodes[0].config.origin_y = current_y;
	}

	for (size_t line_index = 0; line_index < view->nlines; line_index++) {
		buffer_lnode * current_lnode = &view->lnodes[line_index];
		buffer_node * current_node = NULL;
		for (size_t node_index = 0; node_index < current_lnode->count; node_index++) {
			current_node = &current_lnode->nodes[node_index];
			current_node->config.origin_x = current_x;
			current_node->config.origin_y = current_y;
			text_renderer_do(&current_node->config);
			current_x = current_node->config.curr_x;
		}
		current_x = view->scr_x;
		current_y = current_node->config.curr_y;
	}
}

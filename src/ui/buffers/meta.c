#include "minide/ui/buffers/meta.h"
#include "minide/app.h"

#include "minide/debug/border.h"

#include <assert.h>

#define LINES_INITIAL_CAPACITY 3
#define LINES_REALLOC_FACTOR 2

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
	for (size_t line_index = 0; line_index < view->nlines; line_index++) {
		buffer_lnode * current_lnode = &view->lnodes[line_index];
		for (size_t node_index = 0; node_index < current_lnode->count; node_index++) {
			buffer_node * current_node = &current_lnode->nodes[node_index];
			text_renderer_undo(&current_node->config);
		}
		free(current_lnode->nodes);
		current_lnode->count = 0;
		current_lnode->capacity = 0;
	}
	free(view->lnodes);
}

buffer_lnode * buffer_append_line(buffer_view * view, text_render_config config)
{
	if (view->nlines + 1 >= view->lnodes_capacity) {
		view->lnodes_capacity *= LINES_REALLOC_FACTOR;
		view->lnodes = realloc(view->lnodes, sizeof(buffer_lnode) * view->lnodes_capacity);
	}

	buffer_lnode * current = &view->lnodes[view->nlines];

	buffer_lnode_init(current);
	buffer_lnode_add_config(current, config, 0);

	if (view->nlines == 0) { // view->lnodes[0] always is the first line
		current->lines_index_prev = 0;
	}

	if (view->nlines >= 1) {
		view->lnodes[view->nlines - 1].lines_index_next = view->nlines;
		current->lines_index_prev = view->nlines;
	}

	current->lines_index_next = 0;

	// always keep last line index in the first line
	view->lnodes[0].lines_index_prev = view->nlines;
	// always keep line number in the currnet lnode
	current->line_number = view->nlines + 1;

	view->nlines++;

	return current;
}

buffer_lnode * buffer_append_line_str(buffer_view * view, char * utf8_str)
{
	text_render_config config = (text_render_config){
	    .renderer = view->renderer,
	    .utf8_str = utf8_str,
	};

	return buffer_append_line(view, config);
}

void buffer_render(buffer_view * view)
{
	GLfloat current_x = view->ui.x1;
	// forced to use fonts_man_get_font_by_type because no text_renderer_do is called yet
	// this will cause visual inaccuracies if the preferred font is not the applicable to the first line,
	// it is not clear if there is any use in doing anything special here, your fontconfig setup should accomodate
	// correctly to the majority of text you want to renderer
	GLfloat current_y =
	    view->ui.y1 +
	    (GLfloat)(fonts_man_get_font_by_type(view->renderer->font_style, 0)->face->size->metrics.height >> 6);

	if (view->nlines && view->lnodes[0].count) {
		view->lnodes[0].nodes[0].config.origin_x = current_x;
		view->lnodes[0].nodes[0].config.origin_y = current_y;
	}

	// strictly limit the rendering of the text to the quad using opengl scissor test
	GLsizei scissor_width = view->ui.x2 - view->ui.x1;
	GLsizei scissor_height = view->ui.y2 - view->ui.y1;
	if (scissor_width > 0 && scissor_height > 0) {
		glEnable(GL_SCISSOR_TEST);

		// ah yes, glScissor considers x,y from bottom-left
		// https://www.khronos.org/opengl/wiki/Scissor_Test
		GLsizei y = app_config.scr_height - view->ui.y2;
		glScissor(view->ui.x1, y - 1, scissor_width + 1, scissor_height + 1);
	}

	bool only_one_more_line = false;
	size_t lines_rendered = 0;
	size_t lines_to_render = view->nlines;
	buffer_lnode * current_lnode = &view->lnodes[0];
	while (lines_rendered < lines_to_render) {
		buffer_node * current_node = &current_lnode->nodes[0];
		size_t nodes_rendered = 0;
		bool only_one_more_segment = false;
		while (nodes_rendered < current_lnode->count) {
			// reconfigure settings for current node
			current_node->config.renderer = view->renderer;
			current_node->config.origin_x = current_x;
			current_node->config.origin_y = current_y;
			current_node->config.max_x = view->ui.x2;
			current_node->config.max_line_width_chars = view->settings.line_wrap_chars;
			current_node->config.spacing = view->settings.line_spacing;

			text_renderer_do(&current_node->config);
			current_x = current_node->config.curr_x;
			current_y = current_node->config.curr_y;

			current_node = &current_lnode->nodes[current_node->node_index_next];
			nodes_rendered++;
			if (!only_one_more_segment && current_x > view->ui.x2) {
				only_one_more_segment = true;
				continue;
			}
			if (only_one_more_segment) {
				break;
			}
		}
		lines_rendered++;
		current_lnode = &view->lnodes[current_lnode->lines_index_next];
		if (!only_one_more_line && current_y > view->ui.y2) {
			only_one_more_line = true;
			continue;
		}
		if (only_one_more_line) {
			break;
		}
	}

	if (app_config.gl_debug.buffer_scissor_border) {
		debug_border(view->ui.x1, view->ui.y1, view->ui.x2, view->ui.y2);
	}
	glDisable(GL_SCISSOR_TEST);
}

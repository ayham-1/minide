#ifndef BUFFER_META_H
#define BUFFER_META_H

#include "minide/buffers/node.h"
#include "minide/text_renderer.h"

#include <unicode/uchar.h>

typedef struct {
	bool text;
	bool picture;
	bool editable;
	bool code;
	bool terminal;
} buffer_abilities;

typedef struct {
	UChar * name;
	size_t nlines;
	size_t current_line;
	size_t current_coln;
	buffer_abilities abilities;

	// vec_t v_renderers;
	text_renderer_t * renderer;

	buffer_lnode * lnodes;
	size_t lnodes_capacity;

	GLfloat scr_x;
	GLfloat scr_y;

	// TODO(ayham-1): limit size of view
	// maybe have a view struct which holds basic "view" UI data
} buffer_view;

void buffer_init(buffer_view *);
void buffer_clean(buffer_view *);

void buffer_add_renderer(buffer_view *, text_renderer_t);

buffer_lnode * buffer_append_line(buffer_view *, text_render_config config);

void buffer_render_all(buffer_view *);

#endif

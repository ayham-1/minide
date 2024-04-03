#ifndef BUFFER_META_H
#define BUFFER_META_H

#include "minide/text_renderer.h"
#include "minide/ui/buffers/node.h"
#include "minide/ui/view.h"

#include <GL/glew.h>
#include <unicode/uchar.h>

typedef struct {
	bool text;
	bool picture;
	bool editable;
	bool code;
	bool terminal;
} buffer_abilities;

typedef struct {
	int32_t line_wrap_chars;
	int32_t line_spacing;
} buffer_settings;

typedef struct {
	UChar * name;
	size_t nlines;
	buffer_abilities abilities;
	buffer_settings settings;

	size_t current_line;
	size_t current_coln;

	// vec_t v_renderers;
	text_renderer_t * renderer;

	buffer_lnode * lnodes;
	size_t lnodes_capacity;

	view_t ui;

	// TODO(ayham-1): limit size of view
	// maybe have a view struct which holds basic "view" UI data
} buffer_view;

void buffer_init(buffer_view *);
void buffer_clean(buffer_view *);

void buffer_add_renderer(buffer_view *, text_renderer_t);

buffer_lnode * buffer_append_line(buffer_view *, text_render_config config);
buffer_lnode * buffer_append_line_str(buffer_view *, char *);

void buffer_render(buffer_view *);

#endif

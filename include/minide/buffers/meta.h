#ifndef BUFFER_META_H
#define BUFFER_META_H

#include "minide/buffers/node.h"
#include "minide/text_renderer.h"

#include "minide/types/vector.h"

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

	vec_t v_renderers;

	vec_t v_lines;

	double scr_x;
	double scr_y;
} buffer_view;

bool buffer_init(buffer_view *, text_renderer_t initial, double x, double y);
void buffer_clean(buffer_view *);

void buffer_add_renderer(buffer_view *, text_renderer_t);

bool buffer_append_line(buffer_view *, text_render_config config);

void buffer_render(buffer_view *);

#endif

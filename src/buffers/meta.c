#include "minide/buffers/meta.h"

#include <assert.h>

#define BUFFER_INITIAL_RENDERERS_COUNT 3

bool buffer_init(buffer_view * view, text_renderer_t initial, double x, double y)
{
	assert(view);
	// view->t_renderers = malloc(sizeof(buffer_view) * BUFFER_INITIAL_RENDERERS_COUNT);
	vec_create(&view->v_renderers, BUFFER_INITIAL_RENDERERS_COUNT, sizeof(text_renderer_t));

	return true;
};

void buffer_clean(buffer_view * view) {}

bool buffer_append_line(buffer_view * view, text_render_config config) { return true; }

void buffer_render(buffer_view * view) {}

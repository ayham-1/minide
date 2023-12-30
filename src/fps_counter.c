#include "minide/fps_counter.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "minide/font.h"
#include "minide/font_manager.h"

static size_t nbFrames;

static text_renderer_t renderer;
static text_render_config conf = {
    .renderer = &renderer,

    .wrappable = false,
    .max_line_width_chars = 80,
    .base_direction = UBIDI_RTL,

    .str = NULL,

    .origin_x = 0,
    .origin_y = 0,
};

void fps_counter_init(size_t width, size_t height)
{
	text_renderer_init(&renderer, FONT_FAMILY_Monospace, width, height, 12);
}

void fps_counter_render()
{
	byte_t str[21 * 2];
	sprintf((char *)str, "%li fps | %f ms/frame", nbFrames,
		1000.0f / nbFrames);

	conf.str = str;

	text_renderer_do(&conf);
}

void fps_counter_update(size_t frames) { nbFrames = frames; }

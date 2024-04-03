#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "minide/glyph_cache.h"
#include "minide/logger.h"
#include "minide/path.h"

#define PATH "assets/FreeSans.ttf"
#define PATH_BYTES_NUM sizeof(PATH)

#include "minide/app.h"

app_config_t app_config = (app_config_t){
    .max_textures_available = 10,
    .scr_width = 1000,
    .scr_height = 700,
    .scr_target_fps = 1,
    .scr_title = "minide",

    .do_print_frame_ms = false,
    .do_render_frame_ms = false,

    .gl_wrapper_do_close = false,
};

void gl_wrapper_init() {}

void gl_wrapper_render() {}

void gl_wrapper_clean() {}

void glfw_size_callback(int width, int height)
{
	(void)width;
	(void)height;
}

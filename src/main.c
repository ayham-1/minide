#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/glyph_cache.h"
#include "../src/logger.h"
#include "../src/path.h"

#define PATH "assets/FreeSans.ttf"
#define PATH_BYTES_NUM sizeof(PATH)

#include "../src/gl_wrapper.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
gl_wrapper_config_t config = (gl_wrapper_config_t){
    .max_textures_available = 10,
    .scr_width = 1000,
    .scr_height = 700,
    .scr_target_fps = 1,
    .scr_title = "minide",

    .do_print_frame_ms = false,
    .do_render_frame_ms = false,

    .gl_wrapper_do_close = false,
};
#pragma GCC diagnostic pop

void gl_wrapper_init() {}

void gl_wrapper_render() {}

void gl_wrapper_clean() {}

void glfw_size_callback(int width, int height)
{
	(void)width;
	(void)height;
}

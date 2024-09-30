#include <assert.h>

#include "minide/fontconfig.h"
#include "minide/logger.h"

#include "minide/app.h"

app_config_t app_config = (app_config_t){
    .max_textures_available = 10,
    .scr_width = 1000,
    .scr_height = 700,
    .scr_target_fps = 1,
    .scr_title = "test_fonctonfig",

    .do_print_frame_ms = false,
    .do_render_frame_ms = false,

    .gl_wrapper_do_close = false,
};

void gl_wrapper_init()
{
	log_debug("monospace: %s", fc_get_path_by_font(fc_request("monospace")));
	log_debug("serif: %s", fc_get_path_by_font(fc_request("serif")));
	log_debug("sans-serif: %s", fc_get_path_by_font(fc_request("sans-serif")));
	log_debug("emoji: %s", fc_get_path_by_font(fc_request("emoji")));

	fc_holder * holder = fc_request("arabic");
	log_debug("serif-1: %s", fc_get_path_by_font_order(holder, 1));
	log_debug("sans-serif-1: %s", fc_get_path_by_font_order(holder, 1));
	log_debug("monospace-1: %s", fc_get_path_by_font_order(holder, 1));
}

void gl_wrapper_render() { app_config.gl_wrapper_do_close = true; }

void gl_wrapper_clean() {}

void glfw_size_callback(int width, int height) {}

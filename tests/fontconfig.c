#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/fontconfig.h"
#include "../src/logger.h"
#include "../src/path.h"
#include "../src/u8string.h"

#include "../src/gl_wrapper.h"

gl_wrapper_config_t config = (gl_wrapper_config_t){
    .max_textures_available = 10,
    .scr_width = 1000,
    .scr_height = 700,
    .scr_target_fps = 1,
    .scr_title = "test_fonctonfig",

    .do_print_frame_ms = false,
    .do_render_frame_ms = false,

    .gl_wrapper_do_close = false,
};

void gl_wrapper_init() {
    log_debug("monospace: %s", fc_get_path_by_font(fc_request("monospace")));
    log_debug("serif: %s", fc_get_path_by_font(fc_request("serif")));
    log_debug("sans-serif: %s", fc_get_path_by_font(fc_request("sans-serif")));
    log_debug("emoji: %s", fc_get_path_by_font(fc_request("emoji")));
}

void gl_wrapper_render() { config.gl_wrapper_do_close = true; }

void gl_wrapper_clean() {}

void glfw_size_callback(int width, int height) {}

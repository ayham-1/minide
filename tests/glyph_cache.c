#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "minide/glyph_cache.h"
#include "minide/logger.h"
#include "minide/path.h"

#include "minide/font_manager.h"
#include "minide/gl_wrapper.h"

gl_wrapper_config_t config = (gl_wrapper_config_t){
    .max_textures_available = 10,
    .scr_width = 1000,
    .scr_height = 700,
    .scr_target_fps = 1,
    .scr_title = "test_glyph_cache",

    .do_print_frame_ms = false,
    .do_render_frame_ms = false,

    .gl_wrapper_do_close = false,
};

glyph_cache cache;

void gl_wrapper_init() { assert(glyph_cache_create(&cache, fonts_man_get()->monospace[0]->face, 512, 24)); }

void gl_wrapper_render() { config.gl_wrapper_do_close = true; }

void gl_wrapper_clean() { glyph_cache_cleanup(&cache); }

void glfw_size_callback(int width, int height) {}

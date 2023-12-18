#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../src/path.h"
#include "../src/logger.h"
#include "../src/u8string.h"
#include "../src/text_renderer.h"

#define PATH "assets/unifont.ttf"
#define PATH_BYTES_NUM sizeof(PATH)

#include "../src/gl_wrapper.h"

gl_wrapper_config_t config = (gl_wrapper_config_t) {
    .max_textures_available = 10,
    .scr_width = 1000,
    .scr_height = 700,
    .scr_target_fps = 1,
    .scr_title = "test_text_renderer_emoji",

    .do_print_frame_ms = false,
    .do_render_frame_ms = false,

    .gl_wrapper_do_close = false,
};

#define TEST_DATA "😘😘😘🤐🤔🫡🫨😌😔😴"
#define TEST_DATA2 "😘😘😘🤐🤔🫡🫨😌😔😴"

text_renderer_t renderer;
text_render_config conf1;
text_render_config conf2;

void gl_wrapper_init() {
    text_renderer_init(&renderer, FONT_FAMILY_Emoji, config.scr_width, config.scr_height, 24);

    conf1 = (text_render_config) {
        .renderer = &renderer,

        .wrappable = false,
        .max_line_width_chars = 100,
        .base_direction = UBIDI_DEFAULT_LTR,

        .str = (byte_t*) &TEST_DATA,

        .origin_x = 100,
        .origin_y = 100,
    };

    conf2 = (text_render_config) {
        .renderer = &renderer,

        .wrappable = false,
        .max_line_width_chars = 100,
        .base_direction = UBIDI_DEFAULT_LTR,

        .str = (byte_t*) &TEST_DATA2,

        .origin_x = 100,
        .origin_y = 200,
    };
}

void gl_wrapper_render() {
    text_renderer_do(&conf1);
    text_renderer_do(&conf2);

    //config.gl_wrapper_do_close = true;
}

void gl_wrapper_clean() {
    text_renderer_undo(&conf2);

    text_renderer_cleanup(&renderer);
}

void glfw_size_callback(int width, int height) {
   text_renderer_update_window_size(&renderer, width, height);
}

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <unicode/uchar.h>

#include "minide/logger.h"
#include "minide/path.h"
#include "minide/text_renderer.h"
#include "minide/u8string.h"

#include "minide/gl_wrapper.h"

gl_wrapper_config_t config = (gl_wrapper_config_t){
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
#define TEST_DATA2 "hello 😘😘😘🤐 hello world 🤔🫡🫨😌😔😴 world"
#define TEST_DATA_SEQ "##️⃣**️⃣00️⃣11️⃣22️⃣33️⃣44️⃣55️⃣66️⃣77️⃣88️⃣99️⃣10🔟"
#define TEST_DATA_FLAGS "🇩🇪germany 🇬🇧uk 🇵🇸🏁🇸🇯🇹🇷🇸🇾"
#define TEST_DATA_COMBO "🤣😂👩🏻‍🦱🖖🏾👧🏽👩🏻‍🦲"

text_renderer_t renderer;
text_render_config conf1;
text_render_config conf2;
text_render_config conf3;
text_render_config conf4;

void gl_wrapper_init()
{
	text_renderer_init(&renderer, FONT_FAMILY_Monospace, config.scr_width, config.scr_height, 24);

	int offset_x = 10;
	int offset_y = config.scr_height - 35;

	conf1 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 100,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .str = (byte_t *)&TEST_DATA,

	    .origin_x = offset_x,
	    .origin_y = offset_y,
	};

	offset_y -= 25;
	conf2 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 100,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .str = (byte_t *)&TEST_DATA2,

	    .origin_x = offset_x,
	    .origin_y = offset_y,
	};

	offset_y -= 25;
	conf3 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 100,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .str = (byte_t *)&TEST_DATA_SEQ,

	    .origin_x = offset_x,
	    .origin_y = offset_y,
	};

	offset_y -= 25;
	conf3 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 100,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .str = (byte_t *)&TEST_DATA_FLAGS,

	    .origin_x = offset_x,
	    .origin_y = offset_y,
	};

	offset_y -= 25;
	conf4 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 100,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .str = (byte_t *)&TEST_DATA_COMBO,

	    .origin_x = offset_x,
	    .origin_y = offset_y,
	};
}

void gl_wrapper_render()
{
	text_renderer_do(&conf1);
	text_renderer_do(&conf2);
	text_renderer_do(&conf3);
	text_renderer_do(&conf3);
	text_renderer_do(&conf4);

	config.gl_wrapper_do_close = true;
}

void gl_wrapper_clean()
{
	text_renderer_undo(&conf1);
	text_renderer_undo(&conf2);
	text_renderer_undo(&conf3);
	text_renderer_undo(&conf4);

	text_renderer_cleanup(&renderer);
}

void glfw_size_callback(int width, int height) { text_renderer_update_window_size(&renderer, width, height); }

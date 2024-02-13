#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "minide/logger.h"
#include "minide/path.h"
#include "minide/text_renderer.h"
#include "minide/u8string.h"

#define PATH "assets/unifont.ttf"
#define PATH_BYTES_NUM sizeof(PATH)

#include "minide/gl_wrapper.h"

gl_wrapper_config_t config = (gl_wrapper_config_t){
    .max_textures_available = 10,
    .scr_width = 1000,
    .scr_height = 700,
    .scr_target_fps = 1,
    .scr_title = "test_text_renderer",

    .do_print_frame_ms = false,
    .do_render_frame_ms = false,

    .gl_wrapper_do_close = false,
};

#define TEST_DATA                                                                                                      \
	"~`ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopqrstvwxyz!@#$%^&*()_-=+[]{}"                                         \
	"\\|;"                                                                                                         \
	":'"                                                                                                           \
	"\",.<>/?\0"

#define TEST_DATA_ENGLISH_MIXED "hello عالم world"
#define TEST_DATA_ARABIC_MIXED "مرحبا world بالعالم"
#define TEST_DATA_ARABIC_ONLY "مرحبا بالعالم"

#define TEST_DATA_LINE_WRAPPING_STANDARD "WRAP TEST STANDARD 1 WRAP TEST STANDARD 2 WRAP TEST STANDARD 3 #END#"
#define TEST_DATA_LINE_WRAPPING_OVERFLOW                                                                               \
	"12345678901234567890 12345678901234567890# WRAP TEST STANDARD 1 "                                             \
	"WRAP "                                                                                                        \
	"TEST "                                                                                                        \
	"STANDARD 2 WRAP TEST STANDARD 3 #END#"
#define TEST_DATA_LINE_WRAPPING_WRAP_ON_WORD                                                                           \
	"HELLOWORLDTHISISAWRAPONWORDTEST OTHERWISE KNOWN AS A LINE WRAPPING "                                          \
	"TEST "                                                                                                        \
	"#END#"
#define TEST_DATA_LINE_WRAPPING_FORCEFUL                                                                               \
	"HELLOWORLDTHISISAFORCEFULWRAPHELLOWORLDTHISISAFORCEFULWRAPHELLOWORLD"                                         \
	"THIS"                                                                                                         \
	"IS"                                                                                                           \
	"AFORCEFULWRAP#END#"

#define TEST_DATA_CACHE_EXPANSION "HELLO ᙭ WORLD ФϴШՋᏍᓉᔩᗅ˩˨ʯʶΩǞŮŠǅƵҦфъ҂ה੦"

text_renderer_t renderer;
text_render_config conf1;
text_render_config conf2;
text_render_config conf3;
text_render_config conf4;
text_render_config conf5;
text_render_config conf6;
text_render_config conf7;
text_render_config conf8;
text_render_config conf9;

void gl_wrapper_init()
{
	text_renderer_init(&renderer, FONT_FAMILY_Monospace, config.scr_width, config.scr_height, 18);

	conf1 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 100,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .str = (byte_t *)&TEST_DATA,

	    .origin_x = 100,
	    .origin_y = 100,
	};

	conf2 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 100,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .str = (byte_t *)&TEST_DATA_ARABIC_ONLY,

	    .origin_x = 100,
	    .origin_y = 130,
	};

	conf3 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 100,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .str = (byte_t *)&TEST_DATA_ARABIC_MIXED,

	    .origin_x = 100,
	    .origin_y = 150,
	};

	conf4 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 100,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .str = (byte_t *)&TEST_DATA_ENGLISH_MIXED,

	    .origin_x = 100,
	    .origin_y = 170,
	};

	conf5 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 100,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .str = (byte_t *)&TEST_DATA_CACHE_EXPANSION,

	    .origin_x = 100,
	    .origin_y = 190,
	};

	int32_t begin_y = 360;
	conf6 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 20,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .str = (byte_t *)&TEST_DATA_LINE_WRAPPING_STANDARD,

	    .origin_x = 50,
	    .origin_y = begin_y,
	};

	conf7 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 20,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .str = (byte_t *)&TEST_DATA_LINE_WRAPPING_OVERFLOW,

	    .origin_x = 300,
	    .origin_y = begin_y,
	};

	conf8 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 20,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .str = (byte_t *)&TEST_DATA_LINE_WRAPPING_WRAP_ON_WORD,

	    .origin_x = 500,
	    .origin_y = begin_y,
	};

	conf9 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 20,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .str = (byte_t *)&TEST_DATA_LINE_WRAPPING_FORCEFUL,

	    .origin_x = 750,
	    .origin_y = begin_y,
	};
}

void gl_wrapper_render()
{
	text_renderer_do(&conf1);
	text_renderer_do(&conf2);
	text_renderer_do(&conf3);
	text_renderer_do(&conf4);
	text_renderer_do(&conf5);

	// wrap tests
	text_renderer_do(&conf6);
	text_renderer_do(&conf7);
	text_renderer_do(&conf8);
	text_renderer_do(&conf9);

	log_error("test 123");

	config.gl_wrapper_do_close = true;
}

void gl_wrapper_clean()
{
	text_renderer_undo(&conf1);
	text_renderer_undo(&conf2);
	text_renderer_undo(&conf3);
	text_renderer_undo(&conf4);
	text_renderer_undo(&conf5);
	text_renderer_undo(&conf6);

	text_renderer_undo(&conf7);
	text_renderer_undo(&conf8);
	text_renderer_undo(&conf9);

	text_renderer_cleanup(&renderer);
}

void glfw_size_callback(int width, int height) { text_renderer_update_window_size(&renderer, width, height); }

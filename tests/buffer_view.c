#include "minide/text_renderer.h"
#include "minide/ui/buffers/meta.h"

#define PATH "assets/unifont.ttf"
#define PATH_BYTES_NUM sizeof(PATH)

#include "minide/gl_wrapper.h"

gl_wrapper_config_t config = (gl_wrapper_config_t){
    .max_textures_available = 10,
    .scr_width = 1000,
    .scr_height = 700,
    .scr_target_fps = 1,
    .scr_title = "test_buffer_view",

    .do_print_frame_ms = false,
    .do_render_frame_ms = false,

    .gl_wrapper_do_close = false,
};

#define TEST_DATA                                                                                                      \
	"~`ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopqrstvwxyz!@#$%^&*()_-=+[]{}"                                         \
	"\\|;"                                                                                                         \
	":'"                                                                                                           \
	"\",.<>/?\0"
#define TEST_DATA_NEW_LINE "hello world? \nanyone?\nhello ? world?"

text_renderer_t renderer;
text_render_config conf1;
text_render_config conf2;

buffer_view view;

void gl_wrapper_init()
{
	text_renderer_init(&renderer, FONT_FAMILY_Monospace, config.scr_width, config.scr_height, 18);

	conf1 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 100,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .utf8_str = TEST_DATA,
	};

	conf2 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 100,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .utf8_str = TEST_DATA_NEW_LINE,
	};

	view = (buffer_view){
	    .renderer = &renderer,

	    .ui =
		(view_t){
		    .x1 = 200,
		    .y1 = 200,
		},
	};

	buffer_init(&view);
	buffer_append_line(&view, conf1);
	buffer_append_line(&view, conf2);
}

void gl_wrapper_render()
{
	buffer_render_all(&view);
	// text_renderer_do(&conf1);
	// text_renderer_do(&conf2);
	//  config.gl_wrapper_do_close = true;
}

void gl_wrapper_clean()
{
	// text_renderer_undo(&conf1);
	// text_renderer_undo(&conf2);
	buffer_clean(&view);
	text_renderer_cleanup(&renderer);
}

void glfw_size_callback(int width, int height) { text_renderer_update_window_size(&renderer, width, height); }

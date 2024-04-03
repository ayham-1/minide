#include "minide/text_renderer.h"
#include "minide/ui/buffers/meta.h"

#define PATH "assets/unifont.ttf"
#define PATH_BYTES_NUM sizeof(PATH)

#include "minide/app.h"

app_config_t app_config = (app_config_t){
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

buffer_view view1;
buffer_view view2;

void gl_wrapper_init()
{
	text_renderer_init(&renderer, FONT_FAMILY_Monospace, app_config.scr_width, app_config.scr_height, 18);

	conf1 = (text_render_config){
	    .wrappable = false,
	    .max_line_width_chars = 100,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .utf8_str = TEST_DATA,
	};

	conf2 = (text_render_config){
	    .wrappable = false,
	    .max_line_width_chars = 100,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .utf8_str = TEST_DATA_NEW_LINE,
	};

	GLfloat offset_y = 0;

	view1 = (buffer_view){
	    .renderer = &renderer,

	    .settings =
		(buffer_settings){
		    .line_spacing = 0,
		    .line_wrap_chars = 0,
		},

	    .ui =
		(view_t){
		    .x1 = 0,
		    .y1 = offset_y,
		},
	};
	buffer_init(&view1);
	buffer_append_line(&view1, conf1);
	buffer_append_line(&view1, conf2);

	offset_y += 150;
	view2 = (buffer_view){
	    .renderer = &renderer,

	    .settings =
		(buffer_settings){
		    .line_spacing = 0,
		    .line_wrap_chars = 0,
		},

	    .ui =
		(view_t){
		    .x1 = 50,
		    .y1 = offset_y,
		    .x2 = 520,
		    .y2 = offset_y + 84,
		},
	};

	buffer_init(&view2);
	buffer_append_line(&view2, conf1);
	buffer_append_line_str(&view2, "line 2");
	buffer_append_line_str(&view2, "line 3");
	buffer_append_line_str(&view2, "line 4: this line should not be seen fully");
}

void gl_wrapper_render()
{
	buffer_render(&view1);
	app_config.gl_debug.buffer_scissor_border = true;
	buffer_render(&view2);
	app_config.gl_debug.buffer_scissor_border = false;
	//    config.gl_wrapper_do_close = true;
}

void gl_wrapper_clean()
{
	buffer_clean(&view1);
	text_renderer_cleanup(&renderer);
}

void glfw_size_callback(int width, int height) {}

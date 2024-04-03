#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <unicode/uchar.h>

#include "minide/logger.h"
#include "minide/path.h"
#include "minide/text_renderer.h"
#include "minide/u8string.h"

#include "minide/app.h"

app_config_t app_config = (app_config_t){
    .max_textures_available = 10,
    .scr_width = 1000,
    .scr_height = 700,
    .scr_target_fps = 60,
    .scr_title = "test_unicode_emoji",

    .do_print_frame_ms = false,
    .do_render_frame_ms = false,

    .gl_wrapper_do_close = false,
};

text_renderer_t renderer;
text_render_config conf1;

// Function to read file contents into a heap-allocated array
char * read_file(const char * filename, size_t * size)
{
	FILE * file = fopen(filename, "rb");
	if (file == NULL) {
		log_error("Error opening file");
		exit(EXIT_FAILURE);
	}

	// Get the size of the file
	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	rewind(file);

	// Allocate memory for file contents
	char * buffer = malloc(*size);

	// Read file contents into the buffer
	if (fread(buffer, 1, *size, file) != *size) {
		log_error("Error reading file");
		free(buffer);
		fclose(file);
		exit(EXIT_FAILURE);
	}

	fclose(file);
	return buffer;
}

void gl_wrapper_init()
{
	text_renderer_init(&renderer, FONT_FAMILY_Monospace, app_config.scr_width, app_config.scr_height, 24);

	size_t file_size;
	char * file_contents = read_file("assets/emoji-test-15.1.txt", &file_size);
	// printf("%s", file_contents);
	file_contents[55100] = 0; // currently not optimized for large amounts of text

	int offset_x = 10;
	int offset_y = 0;

	conf1 = (text_render_config){
	    .renderer = &renderer,

	    .wrappable = false,
	    .max_line_width_chars = 10000,
	    .base_direction = UBIDI_DEFAULT_LTR,

	    .utf8_str = file_contents,

	    .origin_x = offset_x,
	    .origin_y = offset_y,
	};
}

void gl_wrapper_render()
{
	text_renderer_do(&conf1);
	// conf1.origin_y += 10;

	// config.gl_wrapper_do_close = true;
}

void gl_wrapper_clean()
{
	text_renderer_undo(&conf1);

	text_renderer_cleanup(&renderer);
}

void glfw_size_callback(int width, int height) {}

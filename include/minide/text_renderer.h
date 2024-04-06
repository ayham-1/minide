#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "shaders_util.h"

#include "font_manager.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <cglm/cglm.h>

#include <unicode/ubidi.h>
#include <unicode/ubrk.h>

typedef struct {
	GLuint shaderProgram;
	GLuint attributeCoord;
	GLuint uniformTex;
	GLuint uniformColor;
	GLuint uniformProjection;
	GLuint uniformSingleColor;

	GLuint vbo;
	GLuint ibo;
	GLuint vao;

	size_t font_pixel_size;
	enum FontFamilyStyle font_style;
} text_renderer_t;

typedef struct {
	int32_t start;
	int32_t end;
	bool soft_line;
} line_indices_t;

typedef struct {
	text_renderer_t * renderer;

	bool wrappable;
	int32_t max_line_width_chars;
	GLfloat spacing;

	line_indices_t * lines;
	size_t lines_cnt;

	UBiDiDirection base_direction;
	UBiDi * para;

	char * utf8_str;
	int32_t utf8_sz;

	UChar * utf16_str;
	int32_t utf16_sz;

	UBreakIterator * it_char;
	int32_t char_num;

	GLfloat origin_x, origin_y, max_x;
	GLfloat curr_x, curr_y;
	GLfloat curr_new_line_y_offset;

} text_render_config;

void text_renderer_init(text_renderer_t * renderer, enum FontFamilyStyle font_style, size_t width, size_t height,
			size_t font_pixel_size);
void text_renderer_cleanup(text_renderer_t * renderer);

void text_renderer_update_window_size(text_renderer_t * renderer, int width, int height);

void text_renderer_do(text_render_config * const conf);
void text_renderer_undo(text_render_config * const conf);

void __text_renderer_line(UBiDi * line, text_render_config * const conf, int32_t logical_line_start_offset,
			  UErrorCode * error_code);
void __text_renderer_run(text_render_config * const conf, int32_t logical_start, int32_t logical_limit);
void __text_renderer_new_line(text_render_config * const conf);

void __text_renderer_calculate_lines(text_render_config * const conf);
void __text_renderer_calculate_soft_wraps(text_render_config * const conf, line_indices_t line, int32_t * cnt,
					  line_indices_t * arr_start);
void __text_renderer_calculate_line_char_width(text_render_config * const conf);

void __text_renderer_get_line_break(text_render_config * const conf, int32_t line_number, int32_t * out_logical_start,
				    int32_t * out_logical_end);
#endif

#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "logger.h"
#include "u8string.h"
#include "path.h"
#include "shaders_util.h"
#include "types/vector.h"

#include "glyph_cache.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <cglm/cglm.h>
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

#include <unicode/ubidi.h>
#include <unicode/ubrk.h>

typedef struct {
    glyph_cache gcache;

    GLuint shaderProgram;
    GLuint attributeCoord;
    GLuint uniformTex;
    GLuint uniformColor;
    GLuint uniformProjection;

    GLuint vbo;
    GLuint ibo;
    GLuint vao;

    mat4 projection;
    size_t scr_width;
    size_t scr_height;

    hb_face_t* hb_face;
    hb_font_t* hb_font;
    hb_buffer_t* hb_buf;
    size_t font_pixel_size;
} text_renderer_t;

typedef struct {
    int32_t start;
    int32_t end;
} wrap_run_indices_t;

typedef struct {
    text_renderer_t* renderer;

    bool wrappable;
    int32_t max_line_width_chars;

    wrap_run_indices_t* wrap_runs_dat;
    int32_t wrap_runs_cnt;

    UBiDiDirection base_direction;

    byte_t* str;
    int32_t str_sz;

    UChar* utf16_str;
    int32_t utf16_sz;

    UBreakIterator* it_char;
    int32_t char_num;

    GLfloat origin_x, origin_y;
    GLfloat curr_x, curr_y;
} text_render_config;

void text_renderer_init(text_renderer_t* renderer, path_t font, 
                        size_t width, size_t height, size_t font_pixel_size);
void text_renderer_cleanup(text_renderer_t* renderer);

void text_renderer_update_window_size(text_renderer_t* renderer, int width, int height);

void text_renderer_do(text_render_config* const conf);
void text_renderer_undo(text_render_config* const conf);

void __text_renderer_line(UBiDi* line, text_render_config* const conf,
                          int32_t logical_line_start_offset,
                          UErrorCode* error_code);
void __text_renderer_run(text_render_config* const conf, 
                         int32_t logical_start, int32_t logical_limit);
void __text_renderer_new_line(text_render_config* const conf);

void __text_renderer_calculate_line_wraps(text_render_config* const conf);
void __text_renderer_calculate_line_char_width(text_render_config* const conf);

void __text_renderer_get_line_break(text_render_config* const conf,
                                    int32_t line_number,
                                    int32_t* out_logical_start,
                                    int32_t* out_logical_end);
#endif

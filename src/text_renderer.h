#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "logger.h"
#include "u8string.h"
#include "path.h"
#include "shaders_util.h"

#include "glyph_cache.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <cglm/cglm.h>
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

#include <unicode/ubidi.h>

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
    text_renderer_t* renderer;

    bool wrappable;
    size_t max_line_width_chars;
    UBiDiDirection base_direction;

    byte_t* str;
    size_t str_sz;

    UChar* utf16_str;
    size_t utf16_sz;

    GLfloat origin_x, origin_y;
    GLfloat curr_x, curr_y; // resets after every "do" call
} text_render_config;

void text_renderer_init(text_renderer_t* renderer, path_t font, 
                        size_t width, size_t height, size_t font_pixel_size);
void text_renderer_cleanup(text_renderer_t* renderer);

void text_renderer_update_window_size(text_renderer_t* renderer, int width, int height);

void text_renderer_do(text_render_config* const conf);
void text_renderer_undo(text_render_config* const conf);

void __text_renderer_line(UBiDi* line, text_render_config* const conf,
                          UErrorCode* error_code);
void __text_renderer_run(text_render_config* const conf, 
                         int32_t logical_start, int32_t logical_limit,
                         UBiDiDirection run_direction);

size_t __text_renderer_get_text_width(const byte_t* const str, 
                                      int32_t logical_start,
                                      int32_t logical_end);
void __text_renderer_get_line_break(UBiDi* bidi,
                                    text_render_config* const conf,
                                    int32_t logical_line_start,
                                    int32_t* out_logical_end);
#endif

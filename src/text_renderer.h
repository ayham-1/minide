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
    size_t font_pixel_size;
} text_renderer_t;

void text_renderer_init(text_renderer_t* renderer, path_t font, 
                        size_t width, size_t height, size_t font_pixel_size);
void text_renderer_cleanup(text_renderer_t* renderer);

void text_renderer_line(text_renderer_t* renderer,
                      byte_t* str,
                      GLfloat x, GLfloat y,
                      size_t pixel_size);

void text_renderer_update_window_size(text_renderer_t* renderer, int width, int height);

#endif

#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "logger.h"
#include "u8string.h"
#include "path.h"
#include "shaders_util.h"

#include "glyph_atlas.h"
#include "glyph_cache.h"

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
    glyph_cache gcache;
    glyph_gpu_atlas gatlas;

    GLuint shaderProgram;
    GLuint attributeCoord;
    GLuint uniformTex;
    GLuint uniformColor;

    GLuint vbo;
} text_renderer_t;

void text_renderer_init(text_renderer_t* renderer, path_t font);
void text_renderer_cleanup(text_renderer_t* renderer);

void text_render_line(text_renderer_t* renderer,
                      byte_t* str,
                      size_t x, size_t y,
                      size_t pixel_size);

#endif

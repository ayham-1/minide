#ifndef GLYPH_ATLAS_H
#define GLYPH_ATLAS_H

#include <GL/glew.h>

#include <ft2build.h>
#include FT_GLYPH_H

#include "logger.h"
#include "u8string.h"
#include "types/hash_table.h"

typedef struct {
    float tx;	// x offset of glyph in texture coordinates
    float ty;	// y offset of glyph in texture coordinates
} glyph_info;

typedef struct {
    GLuint textureID;
    GLuint textureObj;

    size_t width, height;
    size_t last_offset_x, last_offset_y;
    size_t last_row_height;

    size_t capacity;
    size_t fullness;
    glyph_info* glyphs;

    hash_table_t* table; // charcode, glyph_info
} glyph_gpu_atlas;

bool glyph_gpu_atlas_build(glyph_gpu_atlas* const atlas,
                           GLuint texID,
                           size_t inital_capacity,
                           u8encode* encodings,
                           FT_Glyph* glyphs); 
bool glyph_gpu_atlas_append(glyph_gpu_atlas* const atlas,
                            u8encode* encoding,
                            FT_Glyph glyph);
bool glyph_gpu_atlas_activate(glyph_gpu_atlas* const atlas,
                              u8encode* charcode);

void glyph_gpu_atla_cleanup(glyph_gpu_atlas* const atlas);

#define ATLAS_MAX_WIDTH 1024

#endif

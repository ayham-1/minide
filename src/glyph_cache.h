#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>

#include <ft2build.h>
#include FT_GLYPH_H

#include "path.h"
#include "u8string.h"
#include "logger.h"

#include "types/hash_table.h"

typedef struct {
    float ax;	// advance.x
    float ay;	// advance.y

    float bw;	// bitmap.width;
    float bh;	// bitmap.height;

    float bl;	// bitmap_left;
    float bt;	// bitmap_top;

    float tx;	// x offset of glyph in texture coordinates
    float ty;	// y offset of glyph in texture coordinates
} glyph_info;

typedef struct {
    GLuint textureID;

    size_t width, height;

    size_t capacity;
    glyph_info* atlas_glyphs;

    hash_table_t* table; // charcode, glyph_info
} glyph_gpu_atlas;

bool glyph_gpu_atlas_build(glyph_gpu_atlas* const atlas,
                           size_t charcode_capacity,
                           wbyte_t* charcodes,
                           FT_Glyph* glyphs); 
bool glyph_gpu_atlas_activate(glyph_gpu_atlas* const atlas,
                              wbyte_t* charcode);

typedef struct {
    /* Freetype */
    FT_Library ft_library;
    FT_Face ft_face;

    /* cache hash table */
    hash_table_t table; // charcode, FT_Glyph
    wbyte_t* table_keys;
    size_t last_key_offset; 
    FT_Glyph* table_data;
    size_t last_data_offset; 

    /* gpu */
    size_t gpu_glyph_cache_sz;
    size_t gpu_glyph_cache_last_empty;
} glyph_cache;

static glyph_cache g_glyph_cache;

bool gc_init(path_t fontPath, size_t capacity,
               size_t pixelSize,
               bool cacheEnglishTypeface);
void gc_cleanup();

FT_Glyph* gc_cache(wbyte_t charID);
FT_Glyph* gc_get_by_index(wbyte_t charID);

#endif

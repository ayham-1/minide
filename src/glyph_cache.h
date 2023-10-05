#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <GL/glew.h>

#include <ft2build.h>
#include FT_GLYPH_H

#include "path.h"
#include "u8string.h"
#include "logger.h"

#include "types/hash_table.h"

typedef struct {
    // https://learnopengl.com/img/in-practice/glyph.png
    float bearing_x;
    float bearing_y;

    float texture_x;
    float texture_y;

    FT_BitmapGlyph bglyph;
} glyph_info;

typedef struct {
    /* Freetype */
    FT_Library ft_library;
    FT_Face ft_face;

    /* cache hash table */
    hash_table_t table; // uint32_t, glyph_info: font glyphid, info
    size_t capacity;
    size_t fullness;
    uint32_t* keys;
    glyph_info* data;

    /* gpu atlas */
    GLuint atexID;
    GLuint atexOBJ;

    size_t awidth, aheight;
    size_t alast_offset_x, alast_offset_y;
    size_t alast_row_height;
} glyph_cache;

bool glyph_cache_init(glyph_cache* cache, 
                      path_t fontPath, size_t capacity,
                      size_t pixelSize,
                      bool cacheEnglishTypeface);
void glyph_cache_cleanup(glyph_cache* cache);

glyph_info* glyph_cache_retrieve(glyph_cache* cache, 
                                 uint32_t glyhid);
glyph_info* glyph_cache_append(glyph_cache* cache, 
                               uint32_t glyphid);

uint64_t __glyph_cache_table_hash(const uint8_t *const key);
bool __glyph_cache_table_entry_cleanup(hash_table_entry_t *entry);
bool __glyph_cache_table_eql_func(const uint8_t *const key1, const uint8_t *const key2);

void __glyph_cache_atlas_build(glyph_cache* cache);
void __glyph_cache_atlas_refill_gpu(glyph_cache* cache);
void __glyph_cache_atlas_append(glyph_cache* cache, 
                                glyph_info* info);

#define ATLAS_MAX_WIDTH 1024

#endif

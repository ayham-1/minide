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
    /* Freetype */
    FT_Library ft_library;
    FT_Face ft_face;

    /* cache hash table */
    hash_table_t table; // charcode, FT_Glyph
    u8encode* table_keys;
    size_t last_key_offset; 
    FT_Glyph* table_data;
    size_t last_data_offset; 
} glyph_cache;

static glyph_cache g_glyph_cache;

bool gc_init(path_t fontPath, size_t capacity,
               size_t pixelSize,
               bool cacheEnglishTypeface);
void gc_cleanup();

FT_Glyph* gc_cache(u8encode encodeID);

#endif

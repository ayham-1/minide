#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "path.h"
#include "u8string.h"
#include "logger.h"

typedef struct {
    GLint textureID; // -1 -> not cached on GPU
    FT_Glyph glyph;
} glyph_t;

typedef struct cache_bucket_t cache_bucket_t;

typedef struct {
    /* Freetype */
    FT_Library ft_library;
    FT_Face ft_face;

    /* cache hash table */
} glyph_cache;

static glyph_cache g_glyph_cache;

bool gc_create(path_t fontPath, size_t capacity,
               size_t pixelSize,
               bool cacheEnglishTypeface);
void gc_cleanup();

glyph_t* gc_cache(wbyte_t charID);
glyph_t* gc_get_by_index(wbyte_t charID);

#endif

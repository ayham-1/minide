#include "glyph_cache.h"

#include <unistd.h>
#include <assert.h>
#define _GNU_SOURCE
#include <sys/mman.h>

static glyph_cache g_glyph_cache;

struct cache_bucket_t {
    size_t sz;
    glyph_t* entries;
};

bool gc_create(path_t font, size_t capacity,
               size_t pixelSize,
               bool cacheEnglishTypeface) {
    if (FT_Init_FreeType(&g_glyph_cache.ft_library)) {
        log_error("failed library initialization.");
        return false;
    }

    if (FT_New_Face(g_glyph_cache.ft_library, (char*)font.fullPath.bytes,
                    0, &g_glyph_cache.ft_face)) {
        log_error("failed font loading.");
        return false;
    }
    FT_Set_Pixel_Sizes(g_glyph_cache.ft_face, 0, pixelSize);

    log_info("loaded %i glyphs. YEEEPEE!", 
             (int)g_glyph_cache.ft_face->num_glyphs);
    
    return true;
}
void gc_cleanup() {
    /* clean individual freetype glyphs */
    FT_Done_Face(g_glyph_cache.ft_face);
    FT_Done_FreeType(g_glyph_cache.ft_library);
}

glyph_t* gc_cache(wbyte_t charID);
glyph_t* gc_get_by_index(wbyte_t charID);

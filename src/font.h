#ifndef FONT_H
#define FONT_H

#include <ft2build.h>
#include FT_GLYPH_H

#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb.h>

#include "path.h"
#include "types/hash_table.h"

#include "fontconfig.h"
#include "glyph_cache.h"

typedef struct {
    FT_Face face;
    hb_font_t *hb;

    fc_holder *fc_holder;

    glyph_cache *caches;
    size_t caches_capacity;
    size_t caches_fullness;

    float scale;
} font_t;

font_t *font_create(FT_Library ft_lib, fc_holder *fc_holder);
void font_clean(font_t *font);

glyph_info *font_get_glyph(font_t *font, uint32_t glyphid, short pixel_size);

glyph_cache *font_create_glyph_cache(font_t *font, short pixel_size);
glyph_cache *font_get_glyph_cache(font_t *font, short pixel_size);

bool font_does_have_charid(font_t *font, uint32_t charid);

void font_set_pixel_size(font_t *font, short pixel_size);

#endif

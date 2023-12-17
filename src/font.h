#ifndef FONT_H
#define FONT_H

#include <ft2build.h>
#include FT_GLYPH_H

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

#include "path.h"
#include "types/hash_table.h"

#include "fontconfig.h"
#include "glyph_cache.h"

typedef struct {
    FT_Face face;
    hb_font_t* hb;

    fc_holder* fc_holder;

    hash_table_t table;
    size_t table_capacity;
    size_t table_fullness;
    size_t* table_keys;
    glyph_cache* table_data;
} font_t;

font_t* font_create(FT_Library ft_lib,
                  fc_holder* fc_holder);
void font_clean(font_t* font);

glyph_info* font_get_glyph(font_t* font, uint32_t glyphid, size_t pixel_size);

glyph_cache* font_create_glyph_cache(font_t* font, size_t pixel_size);
glyph_cache* font_get_glyph_cache(font_t* font, size_t pixel_size);

bool font_does_have_charid(font_t* font, uint32_t charid);

void font_set_pixel_size(font_t* font, size_t pixel_size);

uint64_t __font_table_hash(const uint8_t *const key);
bool __font_table_entry_cleanup(hash_table_entry_t *entry);
bool __font_table_eql_func(const uint8_t *const key1, const uint8_t *const key2);
void __font_table_printer(const hash_table_entry_t* const entry);

#endif

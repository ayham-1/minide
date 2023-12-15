#ifndef FONTS_H
#define FONTS_H

#include <ft2build.h>
#include FT_GLYPH_H

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

#include "path.h"
#include "types/hash_table.h"

#include "fontconfig.h"
#include "glyph_cache.h"

enum FontFamilyStyle {
    Monospace,
    Serif,
    Sans_Serif,
    Emoji,
};

typedef struct {
    FT_Face face;
    hb_font_t* hb_font;

    fc_holder* holder;

    hash_table_t table;
    glyph_cache gcache;
} font_t;

typedef struct {
    FT_Library ft_lib;

    font_t monospace;
    font_t serif;
    font_t sans_serif;
    font_t emoji;
} fonts_manager;

bool fonts_man_init();
void fonts_man_clean();

fonts_manager* fonts_get();

font_t* fonts_get_by_type(enum FontFamilyStyle style);
font_t* fonts_get_by_preference(enum FontFamilyStyle style, uint32_t codepoint);

#endif

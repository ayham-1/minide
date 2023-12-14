#ifndef FONTS_H
#define FONTS_H

#include <ft2build.h>
#include FT_GLYPH_H

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

#include "path.h"
#include "fontconfig.h"

enum FontFamilyStyle {
    Monospace,
    Serif,
    Sans_Serif,
    Emoji,
};

typedef struct {
    FT_Face face;
    hb_face_t* hb_face;
    hb_font_t* hb_font;

    fc_holder* holder;
} font_family;

typedef struct {
    FT_Library ft_lib;

    font_family monospace;
    font_family serif;
    font_family sans_serif;
    font_family emoji;
} fonts_manager;

bool fonts_man_init();
void fonts_man_clean();

fonts_manager* fonts_get();

font_family* fonts_get_by_type(enum FontFamilyStyle style);

#endif

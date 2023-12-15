#include "fonts.h"

#include <assert.h>

#include "path.h"

fonts_manager fonts_man;

bool fonts_man_init() {
    if (FT_Init_FreeType(&fonts_man.ft_lib)) {
        log_error("failed freetype library initialization.");
        return false;
    }

    fonts_man.monospace.holder = fc_request("monospace");
    fonts_man.serif.holder = fc_request("serif");
    fonts_man.sans_serif.holder = fc_request("sans-serif");
    fonts_man.emoji.holder = fc_request("emoji");

    if (FT_New_Face(fonts_man.ft_lib, 
                    fc_get_path_by_font(fonts_man.monospace.holder),
                    0, &fonts_man.monospace.face)) {
        log_error("failed loading monospace font");
        return false;
    }

    if (FT_New_Face(fonts_man.ft_lib, 
                    fc_get_path_by_font(fonts_man.serif.holder),
                    0, &fonts_man.serif.face)) {
        log_error("failed loading serif font");
        return false;
    }

    if (FT_New_Face(fonts_man.ft_lib, 
                    fc_get_path_by_font(fonts_man.sans_serif.holder),
                    0, &fonts_man.sans_serif.face)) {
        log_error("failed loading sans-serif font");
        return false;
    }

    if (FT_New_Face(fonts_man.ft_lib, 
                    fc_get_path_by_font(fonts_man.emoji.holder),
                    0, &fonts_man.emoji.face)) {
        log_error("failed loading emoji font");
        return false;
    }

    assert(FT_HAS_COLOR(fonts_man.emoji.face));

    fonts_man.monospace.hb_font = hb_ft_font_create_referenced(fonts_man.monospace.face);
    fonts_man.serif.hb_font = hb_ft_font_create_referenced(fonts_man.serif.face);
    fonts_man.sans_serif.hb_font = hb_ft_font_create_referenced(fonts_man.sans_serif.face);
    fonts_man.emoji.hb_font = hb_ft_font_create_referenced(fonts_man.emoji.face);

    return true;
}

void fonts_man_clean() {
    hb_font_destroy(fonts_man.monospace.hb_font);
    hb_font_destroy(fonts_man.serif.hb_font);
    hb_font_destroy(fonts_man.sans_serif.hb_font);
    hb_font_destroy(fonts_man.emoji.hb_font);

    FT_Done_Face(fonts_man.monospace.face);
    FT_Done_Face(fonts_man.serif.face);
    FT_Done_Face(fonts_man.sans_serif.face);
    FT_Done_Face(fonts_man.emoji.face);
    FT_Done_FreeType(fonts_man.ft_lib);
}

fonts_manager* fonts_get() {
    return &fonts_man;
}

font_t* fonts_get_by_type(enum FontFamilyStyle style) {
    switch (style) {
        case Monospace: 
            return &fonts_get()->monospace;
        case Serif: 
            return &fonts_get()->serif;
        case Sans_Serif: 
            return &fonts_get()->sans_serif;
        case Emoji: 
            return &fonts_get()->emoji;
    };
    return NULL;
}

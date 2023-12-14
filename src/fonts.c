#include "fonts.h"

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

    hb_blob_t* monospace_blob = hb_blob_create_from_file((char*) fc_get_path_by_font(fonts_man.monospace.holder));
    fonts_man.monospace.hb_face = hb_face_create(monospace_blob, 0);
    fonts_man.monospace.hb_font = hb_ft_font_create(fonts_man.monospace.face, NULL);

    hb_blob_t* serif_blob = hb_blob_create_from_file((char*) fc_get_path_by_font(fonts_man.serif.holder));
    fonts_man.serif.hb_face = hb_face_create(serif_blob, 0);
    fonts_man.serif.hb_font = hb_ft_font_create(fonts_man.serif.face, NULL);

    hb_blob_t* sans_serif_blob = hb_blob_create_from_file((char*) fc_get_path_by_font(fonts_man.sans_serif.holder));
    fonts_man.sans_serif.hb_face = hb_face_create(sans_serif_blob, 0);
    fonts_man.sans_serif.hb_font = hb_ft_font_create(fonts_man.sans_serif.face, NULL);

    hb_blob_t* emoji_blob = hb_blob_create_from_file((char*) fc_get_path_by_font(fonts_man.emoji.holder));
    fonts_man.emoji.hb_face = hb_face_create(emoji_blob, 0);
    fonts_man.emoji.hb_font = hb_ft_font_create(fonts_man.emoji.face, NULL);

    hb_blob_destroy(monospace_blob);
    hb_blob_destroy(serif_blob);
    hb_blob_destroy(sans_serif_blob);
    hb_blob_destroy(emoji_blob);

    return true;
}

void fonts_man_clean() {
    FT_Done_Face(fonts_man.monospace.face);
    FT_Done_Face(fonts_man.serif.face);
    FT_Done_Face(fonts_man.sans_serif.face);
    FT_Done_Face(fonts_man.emoji.face);
    FT_Done_FreeType(fonts_man.ft_lib);
}

fonts_manager* fonts_get() {
    return &fonts_man;
}

font_family* fonts_get_by_type(enum FontFamilyStyle style) {
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

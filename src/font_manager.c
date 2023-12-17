#include "font_manager.h"

#include <assert.h>

#include "path.h"

fonts_manager fonts_man;

bool fonts_man_init() {
    if (FT_Init_FreeType(&fonts_man.ft_lib)) {
        log_error("failed freetype library initialization.");
        return false;
    }

    fonts_man.monospace = font_create(fonts_man.ft_lib, fc_request("monospace"));
    fonts_man.serif = font_create(fonts_man.ft_lib, fc_request("serif"));
    fonts_man.sans_serif = font_create(fonts_man.ft_lib, fc_request("sans_serif"));
    fonts_man.emoji = font_create(fonts_man.ft_lib, fc_request("emoji"));

    return true;
}

void fonts_man_clean() {
    font_clean(fonts_man.monospace);
    font_clean(fonts_man.serif);
    font_clean(fonts_man.sans_serif);
    font_clean(fonts_man.emoji);

    FT_Done_FreeType(fonts_man.ft_lib);
}

fonts_manager* fonts_man_get() {
    return &fonts_man;
}

font_t* fonts_man_get_font_by_type(enum FontFamilyStyle style) {
    switch (style) {
        case FONT_FAMILY_Monospace:
            return fonts_man_get()->monospace;
        case FONT_FAMILY_Serif:
            return fonts_man_get()->serif;
        case FONT_FAMILY_Sans_Serif:
            return fonts_man_get()->sans_serif;
        case FONT_FAMILY_Emoji:
            return fonts_man_get()->emoji;
    };
    return NULL;
}

#include "fontconfig.h"

#include "logger.h"

#include <assert.h>
#include <malloc.h>

#define INITIAL_PATS_SIZE 10

static FontState fonts;

void fc_init() {
    assert(!fonts.config);

    fonts.config = FcInitLoadConfigAndFonts();
    fonts.sz = 0;
    fonts.capacity = 0;

    log_info("initialized fontconfig library of version %i", FcGetVersion());
}

void fc_clean() {
    assert(fonts.config);

    for (size_t i = 0; i < fonts.capacity; i++) {
        //FcPatternDestroy(fonts.list[i].pattern);
        FcPatternDestroy(fonts.list[i].matched_font);
    }

    FcConfigDestroy(fonts.config);
    FcFini();
}

FontHolder* fc_request(char* font_name) {
    if (fonts.sz == 0) {
        fonts.sz = INITIAL_PATS_SIZE;
        fonts.list = malloc(sizeof(FontHolder) * fonts.sz);
    }

    if (fonts.capacity + 1 >= fonts.sz) {
        fonts.sz += INITIAL_PATS_SIZE;
        fonts.list = realloc(fonts.list, sizeof(FontHolder) * fonts.sz);
    }

    FcPattern* pat = fonts.list[fonts.capacity].pattern;
    pat = FcNameParse((const FcChar8*) font_name);

    FcConfigSubstitute(fonts.config, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);

    FcResult result;
    fonts.list[fonts.capacity].matched_font = FcFontMatch(fonts.config, pat, &result);

    if (result != FcResultMatch) {
        log_error("failed to resolve font name pattern: %s", font_name);
        switch(result) {
            case FcResultNoMatch:
                log_error("error: FcResultNoMatch");
                break;
            case FcResultTypeMismatch:
                log_error("error: FcResultTypeMismatch");
                break;
            case FcResultNoId:
                log_error("error: FcResultNoId");
                break;
            case FcResultOutOfMemory:
                log_error("error: FcResultOutOfMemory");
                break;
            default:
                log_error("unkown error");
        };
        return NULL;
    }

    FcChar8* file = NULL;
    if (FcPatternGetString(fonts.list[fonts.capacity].matched_font, 
                           FC_FILE, 0, &file) != FcResultMatch) {
        log_error("unable to get file path from already matched file pattern");
        file = NULL;
    }
    fonts.list[fonts.capacity].matched_font_path = file;
    return &fonts.list[fonts.capacity++];
}

char* fc_get_path_by_font(FontHolder* holder) {
    assert(holder);

    return (char*) holder->matched_font_path;
}

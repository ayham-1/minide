#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "path.h"

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
    FT_Library library;
    FT_Face fontFace;

} text_renderer_state_t;

void text_renderer_init(path_t* font);
void text_renderer_cleanup(path_t* font);

#endif

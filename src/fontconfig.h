#ifndef FONT_CONFIG_H
#define FONT_CONFIG_H

#include <unistd.h>
#include <fontconfig/fontconfig.h>

typedef struct {
    FcPattern* pattern;

    FcPattern* matched_font;
    FcChar8* matched_font_path;
} FontHolder;

typedef struct {
    size_t sz;
    size_t capacity;
    FontHolder* list;

    FcConfig* config;
} FontState;

void fc_init();
void fc_clean();

FontHolder* fc_request(char* font_pat);

char* fc_get_path_by_font(FontHolder* holder);

#endif

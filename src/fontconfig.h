#ifndef FONT_CONFIG_H
#define FONT_CONFIG_H

#include <unistd.h>
#include <fontconfig/fontconfig.h>

typedef struct {
    FcPattern* pattern;

    FcPattern* matched_font;
    FcChar8* matched_font_path;
} fc_holder;

typedef struct {
    size_t sz;
    size_t capacity;
    fc_holder* list;

    FcConfig* config;
} fc_state;

void fc_init();
void fc_clean();

fc_holder* fc_request(char* font_pat);

char* fc_get_path_by_font(fc_holder* holder);

#endif

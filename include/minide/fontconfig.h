#ifndef FONT_CONFIG_H
#define FONT_CONFIG_H

#include <fontconfig/fontconfig.h>
#include <unistd.h>

typedef struct {
	FcPattern * pattern;

	size_t matched_fonts_n;
	FcFontSet * matched_fonts;
	FcChar8 ** matched_fonts_paths;
} fc_holder;

typedef struct {
	size_t sz;
	size_t capacity;
	fc_holder * list;

	FcConfig * config;
} fc_state;

void fc_init();
void fc_clean();

fc_holder * fc_request(char * font_pat);

char * fc_get_path_by_font(fc_holder * holder);
char * fc_get_path_by_font_order(fc_holder * holder, size_t n);

#endif

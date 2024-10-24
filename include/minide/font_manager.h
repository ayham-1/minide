#ifndef FONT_MAN_H
#define FONT_MAN_H

#include <ft2build.h>
#include FT_GLYPH_H

#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb.h>

#include "minide/path.h"
#include "minide/types/hash_table.h"

#include "fontconfig.h"
#include "glyph_cache.h"

#include "font.h"

enum FontFamilyStyle {
	FONT_FAMILY_Monospace,
	FONT_FAMILY_Serif,
	FONT_FAMILY_Sans_Serif,
	FONT_FAMILY_Emoji,
};

typedef struct {
	FT_Library ft_lib;

	font_t ** monospace;
	int monospace_count;

	font_t ** serif;
	int serif_count;

	font_t ** sans_serif;
	int sans_count;

	font_t ** emoji;
	int emoji_count;
} fonts_manager;

bool fonts_man_init();
void fonts_man_clean();

fonts_manager * fonts_man_get();

font_t * fonts_man_get_font_by_type(enum FontFamilyStyle style, int index);
int fonts_man_get_font_num_by_type(enum FontFamilyStyle sytle);

int __fonts_man_safe_create_fonts(int count, font_t ** list, fc_holder * holder);

#endif

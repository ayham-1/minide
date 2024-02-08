#include "minide/font_manager.h"

#include <assert.h>

#include <freetype/ftlcdfil.h>

#include "minide/path.h"

fonts_manager fonts_man;

bool fonts_man_init()
{
	if (FT_Init_FreeType(&fonts_man.ft_lib)) {
		log_error("failed freetype library initialization.");
		return false;
	}
	FT_Library_SetLcdFilter(fonts_man.ft_lib, FT_LCD_FILTER_DEFAULT);

	fc_holder * monospace = fc_request("monospace");
	fc_holder * serif = fc_request("serif");
	fc_holder * sans_serif = fc_request("sans_serif");
	fc_holder * emoji = fc_request("emoji");

	fonts_man.monospace_count = monospace->matched_fonts_n;
	fonts_man.monospace = calloc(fonts_man.monospace_count, sizeof(font_t *));
	log_info("initialized %i available monospace fonts", fonts_man.monospace_count);
	fonts_man.monospace_count =
	    __fonts_man_safe_create_fonts(fonts_man.monospace_count, fonts_man.monospace, monospace);

	fonts_man.serif_count = serif->matched_fonts_n;
	fonts_man.serif = calloc(fonts_man.serif_count, sizeof(font_t *));
	log_info("initialized %i available serif fonts", fonts_man.serif_count);
	fonts_man.serif_count = __fonts_man_safe_create_fonts(fonts_man.serif_count, fonts_man.serif, serif);

	fonts_man.sans_count = sans_serif->matched_fonts_n;
	fonts_man.sans_serif = calloc(fonts_man.serif_count, sizeof(font_t *));
	log_info("initialized %i available sans_serif fonts", fonts_man.sans_count);
	fonts_man.sans_count = __fonts_man_safe_create_fonts(fonts_man.sans_count, fonts_man.sans_serif, sans_serif);

	fonts_man.emoji_count = emoji->matched_fonts_n;
	fonts_man.emoji = calloc(fonts_man.emoji_count, sizeof(font_t *));
	log_info("initialized %i available emoji fonts", fonts_man.emoji_count);
	fonts_man.emoji_count = __fonts_man_safe_create_fonts(fonts_man.emoji_count, fonts_man.emoji, emoji);

	log_info("first priority monospace: %s", fc_get_path_by_font(monospace));
	log_info("monospace count: %i", fonts_man.monospace_count);
	log_info("first priority serif: %s", fc_get_path_by_font(serif));
	log_info("serif count: %i", fonts_man.serif_count);
	log_info("first priority sans-serif: %s", fc_get_path_by_font(sans_serif));
	log_info("sans-serif count: %i", fonts_man.sans_count);
	log_info("first priority emoji: %s", fc_get_path_by_font(emoji));
	log_info("emoji count: %i", fonts_man.emoji_count);

	return true;
}

void fonts_man_clean()
{
	for (int i = 0; i < fonts_man_get()->monospace_count; i++)
		font_clean(fonts_man.monospace[i]);

	for (int i = 0; i < fonts_man_get()->serif_count; i++)
		font_clean(fonts_man.serif[i]);

	for (int i = 0; i < fonts_man_get()->sans_count; i++)
		font_clean(fonts_man.sans_serif[i]);

	for (int i = 0; i < fonts_man_get()->emoji_count; i++)
		font_clean(fonts_man.emoji[i]);

	FT_Done_FreeType(fonts_man.ft_lib);
}

fonts_manager * fonts_man_get() { return &fonts_man; }

font_t * fonts_man_get_font_by_type(enum FontFamilyStyle style, int index)
{
	switch (style) {
	case FONT_FAMILY_Monospace:
		return fonts_man_get()->monospace[index];
	case FONT_FAMILY_Serif:
		return fonts_man_get()->serif[index];
	case FONT_FAMILY_Sans_Serif:
		return fonts_man_get()->sans_serif[index];
	case FONT_FAMILY_Emoji:
		return fonts_man_get()->emoji[index];
	};
	return NULL;
}

int fonts_man_get_font_num_by_type(enum FontFamilyStyle style)
{
	switch (style) {
	case FONT_FAMILY_Monospace:
		return fonts_man_get()->monospace_count;
	case FONT_FAMILY_Serif:
		return fonts_man_get()->serif_count;
	case FONT_FAMILY_Sans_Serif:
		return fonts_man_get()->sans_count;
	case FONT_FAMILY_Emoji:
		return fonts_man_get()->emoji_count;
	};
	return -1;
}

int __fonts_man_safe_create_fonts(int count, font_t ** list, fc_holder * holder)
{
	int ignored_fonts = 0;
	for (int i = 0; i < count; i++) {
		font_t * font_in_question = font_create(fonts_man.ft_lib, holder, i);
		if (FT_HAS_SVG(font_in_question->face) && !FT_HAS_COLOR(font_in_question->face)) {
			log_warn("refusing font: %s", holder->matched_fonts_paths[font_in_question->fc_holder_index]);
			log_warn("reason: not using SVG font, as it is a pain in the ass to implement.");
			log_warn(
			    "please switch to a non-SVG font, for humanity's sake, for my sake, and for your sake :D");
			log_warn("if you are using SVG emoji font, make sure to use COLRv0-supported font");
			ignored_fonts++;
			continue;
		}

		list[i] = font_in_question;
	}

	return count - ignored_fonts;
}

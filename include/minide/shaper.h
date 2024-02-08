#ifndef SHAPER_H
#define SHAPER_H

#include "logger.h"

#include "font.h"
#include "font_manager.h"

#include <harfbuzz/hb.h>
#include <unicode/uchar.h>

typedef struct {
	font_t * font;
	float scale;

	hb_glyph_info_t * glyph_infos;
	hb_glyph_position_t * glyph_pos;
	size_t glyph_count;
} shaper_font_run_t;

typedef struct {
	size_t pixel_size;
	enum FontFamilyStyle preferred_style;
	bool do_style_falllback;
	bool do_font_fallback;

	shaper_font_run_t * runs;
	size_t runs_capacity;
	size_t runs_fullness;

	hb_buffer_t * buffer;
	UChar * utf16_str;
	int32_t logical_length;
} shaper_holder;

void shaper_do(shaper_holder * holder);
void shaper_undo(shaper_holder * holder);
void shaper_free(shaper_holder * holder);

void shaper_do_segment(shaper_holder * holder, int32_t start, int32_t end, font_t * font);

void __shaper_add_run(shaper_holder * holder, shaper_font_run_t run);
shaper_font_run_t __shaper_make_run(font_t * font, hb_glyph_info_t * restrict info, hb_glyph_position_t * restrict pos,
				    int glyph_count);
void __shaper_clean_run(shaper_font_run_t * run);

#endif

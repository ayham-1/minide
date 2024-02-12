#include "minide/shaper.h"

#include <assert.h>

#include <unicode/ustring.h>
#include <unicode/utf16.h>

void shaper_do(shaper_holder * holder)
{
	assert(holder);
	assert(holder->pixel_size);
	assert(holder->utf16_str);
	assert(holder->logical_length);

	holder->runs_capacity = 3;
	holder->runs_fullness = 0;
	holder->runs = calloc(holder->runs_capacity, sizeof(shaper_font_run_t));

	holder->buffer = hb_buffer_create();
	assert(hb_buffer_allocation_successful(holder->buffer));

	size_t run_end = 0;
	while (run_end < holder->logical_length) {
		bool is_emoji_run = __shaper_is_char_emoji(holder->utf16_str, run_end, holder->logical_length);
		int run_start = run_end;

		while (run_end < holder->logical_length &&
		       __shaper_is_char_emoji(holder->utf16_str, run_end, holder->logical_length) == is_emoji_run) {
			UChar32 cpoint;
			U16_NEXT(holder->utf16_str, run_end, holder->logical_length, cpoint);
		}

		if (is_emoji_run) {
			enum FontFamilyStyle old_style = holder->preferred_style;
			holder->preferred_style = FONT_FAMILY_Emoji;
			__shaper_do_segment_best(holder, run_start, run_end);
			holder->preferred_style = old_style;
		} else {
			__shaper_do_segment_best(holder, run_start, run_end);
		}
	}
}

void shaper_undo(shaper_holder * holder)
{
	for (size_t i = 0; i < holder->runs_fullness; i++) {
		__shaper_clean_run(&holder->runs[i]);
	}

	// free(holder->runs);
	holder->runs_fullness = 0;
	// holder->runs_capacity = 0;
	hb_buffer_destroy(holder->buffer);
}

void shaper_free(shaper_holder * holder)
{
	for (size_t i = 0; i < holder->runs_fullness; i++) {
		__shaper_clean_run(&holder->runs[i]);
	}
	free(holder->runs);
	holder->runs_fullness = 0;
	holder->runs_capacity = 0;
	hb_buffer_destroy(holder->buffer);
}

void __shaper_do_segment_best(shaper_holder * holder, int32_t start, int32_t end)
{
	hb_buffer_reset(holder->buffer);
	hb_buffer_clear_contents(holder->buffer);

	int min_notdefs = INT_MAX;
	font_t * best_font = fonts_man_get_font_by_type(holder->preferred_style, 0);
	for (int i = 0; i < fonts_man_get_font_num_by_type(holder->preferred_style); i++) {
		font_t * subrun_font = fonts_man_get_font_by_type(holder->preferred_style, i);

		font_set_pixel_size(subrun_font, holder->pixel_size);

		hb_buffer_reset(holder->buffer);
		hb_buffer_clear_contents(holder->buffer);

		hb_buffer_add_utf16(holder->buffer, (uint16_t *)holder->utf16_str, end - start, start, end - start);

		hb_buffer_guess_segment_properties(holder->buffer);

		hb_shape(subrun_font->hb, holder->buffer, NULL, 0);

		unsigned int glyph_count;
		hb_glyph_info_t * glyph_infos = hb_buffer_get_glyph_infos(holder->buffer, &glyph_count);

		int current_notdef = 0;
		for (int j = 0; j < glyph_count; j++) {
			if (glyph_infos[j].codepoint == 0)
				current_notdef++;
		}

		if (current_notdef < min_notdefs) {
			min_notdefs = current_notdef;
			best_font = subrun_font;
		}

		if (current_notdef == 0 || !holder->do_font_fallback)
			break;

		if (!holder->do_font_fallback)
			break;
	}

	hb_buffer_reset(holder->buffer);
	hb_buffer_clear_contents(holder->buffer);
	hb_buffer_add_utf16(holder->buffer, (uint16_t *)holder->utf16_str, end - start, start, end - start);
	hb_buffer_guess_segment_properties(holder->buffer);
	hb_shape(best_font->hb, holder->buffer, NULL, 0);

	unsigned int glyph_count;
	hb_glyph_info_t * best_glyph_infos = hb_buffer_get_glyph_infos(holder->buffer, &glyph_count);

	hb_glyph_position_t * best_glyph_pos = hb_buffer_get_glyph_positions(holder->buffer, &glyph_count);

	__shaper_add_run(holder, __shaper_make_run(best_font, best_glyph_infos, best_glyph_pos, glyph_count));
}

void __shaper_add_run(shaper_holder * holder, shaper_font_run_t run)
{
	if (holder->runs_fullness + 1 >= holder->runs_capacity) {
		holder->runs =
		    (shaper_font_run_t *)realloc(holder->runs, 2 * holder->runs_capacity * sizeof(shaper_font_run_t));
		holder->runs_capacity *= 2;
		log_info("shaper runs full, attempted realloc");
	}

	holder->runs[holder->runs_fullness++] = run;
}

shaper_font_run_t __shaper_make_run(font_t * font, hb_glyph_info_t * restrict info, hb_glyph_position_t * restrict pos,
				    int glyph_count)
{
	shaper_font_run_t run;

	run.font = font;
	run.scale = font->scale;
	run.glyph_count = glyph_count;

	run.glyph_infos = (hb_glyph_info_t *)calloc(run.glyph_count, sizeof(hb_glyph_info_t));
	run.glyph_pos = (hb_glyph_position_t *)calloc(run.glyph_count, sizeof(hb_glyph_position_t));

	memcpy(run.glyph_infos, info, run.glyph_count * sizeof(hb_glyph_info_t));
	memcpy(run.glyph_pos, pos, run.glyph_count * sizeof(hb_glyph_position_t));

	return run;
}

void __shaper_clean_run(shaper_font_run_t * run)
{
	free(run->glyph_infos);
	free(run->glyph_pos);
	run->glyph_count = 0;
}

bool __shaper_is_char_emoji(UChar * c, size_t offset, size_t length)
{
	UChar32 cpoint;
	U16_NEXT(c, offset, length, cpoint);

	// log_var(cpoint);
	// log_var(u_hasBinaryProperty(cpoint, UCHAR_EMOJI));
	// log_var(u_hasBinaryProperty(cpoint, UCHAR_BASIC_EMOJI));
	// log_var(u_hasBinaryProperty(cpoint, UCHAR_EMOJI_COMPONENT));
	// log_var(u_hasBinaryProperty(cpoint, UCHAR_EMOJI_PRESENTATION));
	// log_var(u_hasBinaryProperty(cpoint, UCHAR_EMOJI_MODIFIER));
	// log_var(u_hasBinaryProperty(cpoint, UCHAR_EMOJI_MODIFIER_BASE));
	// log_var(u_hasBinaryProperty(cpoint, UCHAR_EMOJI_KEYCAP_SEQUENCE));
	// log_var(u_hasBinaryProperty(cpoint, UCHAR_EXTENDED_PICTOGRAPHIC));
	// log_var(u_hasBinaryProperty(cpoint, UCHAR_RGI_EMOJI));
	// log_var(u_hasBinaryProperty(cpoint, UCHAR_REGIONAL_INDICATOR));
	// log_var(u_hasBinaryProperty(cpoint, UCHAR_RGI_EMOJI_MODIFIER_SEQUENCE));
	// log_var(u_hasBinaryProperty(cpoint, UCHAR_RGI_EMOJI_TAG_SEQUENCE));
	// log_var(u_hasBinaryProperty(cpoint, UCHAR_RGI_EMOJI_ZWJ_SEQUENCE));
	// log_var(u_hasBinaryProperty(cpoint, UCHAR_RGI_EMOJI_FLAG_SEQUENCE));

	if (u_hasBinaryProperty(cpoint, UCHAR_EMOJI_KEYCAP_SEQUENCE)) {
		return true;
	}

	if (u_hasBinaryProperty(cpoint, UCHAR_EMOJI_PRESENTATION)) {
		return true;
	}

	if (u_hasBinaryProperty(cpoint, UCHAR_EMOJI_MODIFIER_BASE)) {
		return true;
	}

	if (u_hasBinaryProperty(cpoint, UCHAR_EMOJI_MODIFIER)) {
		return true;
	}

	if (u_hasBinaryProperty(cpoint, UCHAR_EXTENDED_PICTOGRAPHIC)) {
		return true;
	}

	if (u_hasBinaryProperty(cpoint, UCHAR_RGI_EMOJI_FLAG_SEQUENCE)) {
		return true;
	}

	if (u_hasBinaryProperty(cpoint, UCHAR_RGI_EMOJI_MODIFIER_SEQUENCE)) {
		return true;
	}

	if (u_hasBinaryProperty(cpoint, UCHAR_RGI_EMOJI_ZWJ_SEQUENCE)) {
		return true;
	}

	if (u_hasBinaryProperty(cpoint, UCHAR_RGI_EMOJI_TAG_SEQUENCE)) {
		return true;
	}

	if (u_hasBinaryProperty(cpoint, UCHAR_RGI_EMOJI)) {
		return true;
	}

	if (u_hasBinaryProperty(cpoint, UCHAR_REGIONAL_INDICATOR)) {
		return true;
	}

	if (u_hasBinaryProperty(cpoint, UCHAR_EMOJI) && u_hasBinaryProperty(cpoint, UCHAR_BASIC_EMOJI) &&
	    !u_hasBinaryProperty(cpoint, UCHAR_EMOJI_COMPONENT)) {
		return true; // basic emoji
	}

	if (u_hasBinaryProperty(cpoint, UCHAR_EMOJI) && u_hasBinaryProperty(cpoint, UCHAR_EMOJI_COMPONENT) &&
	    !u_hasBinaryProperty(cpoint, UCHAR_BASIC_EMOJI)) {
		U16_NEXT(c, offset, length, cpoint);
		if (u_hasBinaryProperty(cpoint,
					UCHAR_EMOJI_COMPONENT) /*&& !u_hasBinaryProperty(cpoint, UCHAR_EMOJI)*/) {
			return true; // sequence emoji start
		}
	}

	if (u_hasBinaryProperty(cpoint, UCHAR_EMOJI_COMPONENT) && !u_hasBinaryProperty(cpoint, UCHAR_EMOJI) &&
	    !u_hasBinaryProperty(cpoint, UCHAR_BASIC_EMOJI)) {
		return true; // subsequent code points in a sequence
	}

	return false;
}

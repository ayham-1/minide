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
		int run_start = run_end;
		bool is_emoji_run = __shaper_is_char_emoji(holder->utf16_str, run_end, holder->logical_length, NULL);

		size_t new_run_end = 0;
		while (run_end < holder->logical_length &&
		       __shaper_is_char_emoji(holder->utf16_str, run_end, holder->logical_length, &new_run_end) ==
			   is_emoji_run) {
			// __shaper_is_char_emoji() modifies run_end, advancing it,
			// this makes it so that emoji sequences are not mid-sliced into segments
			run_end = new_run_end;
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

#define EMOJI_AS_TEXT_SELECTOR 0xFE0E
#define EMOJI_AS_EMOJI_SELECTOR 0xFE0F

#define EMOJI_KEYCAP_MOD_1 EMOJI_AS_EMOJI_SELECTOR
#define EMOJI_KEYCAP_MOD_2 0x20E3

bool __shaper_is_char_emoji(UChar * c, size_t offset, size_t length, size_t * new_offset)
{
	/*
	 * (ayham-1):
	 * this function tests against multiple characters for emoji sequences,
	 * as such U16_NEXT() will be called multiple times. However, this function
	 * returns the valid offset right after the last emoji in the string. As such,
	 * U16_NEXT()'s operation on the offset *will* need to be "reverted".
	 * new_offset only used if it is not NULL, and is set to the latest emoji
	 * logical item
	 * */

	bool answer = false;
	bool emoji_character = false;
	bool emoji_modifier_sequence = false;
	bool emoji_presentation_sequence = false;

	UChar32 c1, c2;
	U16_NEXT(c, offset, length, c1);

	if (u_hasBinaryProperty(c1, UCHAR_BASIC_EMOJI) || u_hasBinaryProperty(c1, UCHAR_EXTENDED_PICTOGRAPHIC)) {
		U16_GET(c, 0, offset, length, c2);
		if (EMOJI_AS_TEXT_SELECTOR == c2) {
			U16_NEXT(c, offset, length, c2);
			answer = false;
		} else {
			if (EMOJI_AS_EMOJI_SELECTOR == c2) {
				U16_NEXT(c, offset, length, c2);
				emoji_presentation_sequence = true;
			}
			answer = true;
			emoji_character = true;
		}
	} else if (u_hasBinaryProperty(c1, UCHAR_EMOJI_MODIFIER_BASE)) {
		U16_GET(c, 0, offset, length, c2);
		if (u_hasBinaryProperty(c2, UCHAR_EMOJI_MODIFIER)) {
			answer = true;
			emoji_modifier_sequence = true;
			do {
				U16_NEXT(c, offset, length, c2);
				// U16_GET(c, 0, offset, length, c2);
			} while (u_hasBinaryProperty(c2, UCHAR_EMOJI_MODIFIER));
		}
	} else if ((c1 <= 0x39 && c1 >= 0x30) || c1 == 0x2a || c1 == 0x23) {
		U16_GET(c, 0, offset, length, c2);
		if (EMOJI_KEYCAP_MOD_1 == c2) {
			U16_GET(c, 0, offset + 1, length, c2);
			if (EMOJI_KEYCAP_MOD_2 == c2) {
				U16_NEXT(c, offset, length, c2);
				U16_NEXT(c, offset, length, c2);
				answer = true;
			}
		}
	} else if (u_hasBinaryProperty(c1, UCHAR_REGIONAL_INDICATOR)) {
		U16_GET(c, 0, offset, length, c2);
		if (u_hasBinaryProperty(c2, UCHAR_REGIONAL_INDICATOR)) {
			U16_NEXT(c, offset, length, c2);
		}
		answer = true;
	}

	if (emoji_character || emoji_modifier_sequence || emoji_presentation_sequence) {
#define TAG_SPEC_BEGIN 0xE0020
#define TAG_SPEC_END 0xE007E
#define ZWJ_ELEM 0x200d
		U16_GET(c, 0, offset, length, c2);
		answer = true;
		if (0xE007F == c2 || (c2 <= TAG_SPEC_END && c2 >= TAG_SPEC_BEGIN)) {
			// tag sequences
			do {
				U16_NEXT(c, offset, length, c2);
			} while (0xE007F == c2 || (c2 <= TAG_SPEC_END && c2 >= TAG_SPEC_BEGIN));
		} else if (ZWJ_ELEM == c2) {
			// ZWJ sequences
			U16_NEXT(c, offset, length, c2);
		}
	}

	if (NULL != new_offset)
		*new_offset = offset;
	return answer;
}

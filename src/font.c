#include "minide/font.h"

#include <assert.h>
#include <limits.h>

font_t * font_create(FT_Library ft_lib, fc_holder * fc_holder, int fc_holder_index)
{
	font_t * result = malloc(sizeof(font_t));

	result->fc_holder = fc_holder;
	result->fc_holder_index = fc_holder_index;

	if (FT_New_Face(ft_lib, fc_get_path_by_font_order(fc_holder, result->fc_holder_index), 0, &result->face)) {
		log_error("failed font creation");
		return NULL;
	}

	result->hb = hb_ft_font_create_referenced(result->face);

	result->caches_capacity = 3;
	result->caches_fullness = 0;
	result->caches = calloc(result->caches_capacity, sizeof(glyph_cache));

	return result;
}

void font_clean(font_t * font)
{
	hb_font_destroy(font->hb);
	FT_Done_Face(font->face);

	free(font);
}

bool font_does_have_charid(font_t * font, uint32_t charid) { return FT_Get_Char_Index(font->face, charid); }

void font_set_pixel_size(font_t * font, short pixel_size)
{
	/*
	 * This function changes the size of FT_Face and hb_font_t,
	 * it is done centrally here as a way to connect harfbuzz and freetype,
	 * font_t holds glyph_cache per pixel_size, however, the switch between
	 * them is not done explicitly in glyph_cache but rather here. And the
	 * configuration of correct size is done here by FT_Set_Pixel_Sizes()
	 * and hb_font_changed()
	 *
	 * To correctly use this function, call before doing hb_shape() -
	 * shaper.h does this.
	 */
	if (FT_IS_SCALABLE(font->face)) {
		FT_Set_Pixel_Sizes(font->face, 0, pixel_size);
		font->scale = 1.0f;
	} else {
		// pick closest in font->face->available_sizes[]
		short distance = SHRT_MAX;
		int index = 0;
		for (FT_Int i = 0; i < font->face->num_fixed_sizes; i++) {
			short new_distance = abs(font->face->available_sizes[i].height - (FT_Short)pixel_size);
			if (distance >= new_distance) {
				distance = new_distance;
				index = i;
			}
		}

		FT_Select_Size(font->face, index);
		font->scale = ((float)pixel_size) / ((float)font->face->available_sizes[index].height);
	}

	hb_font_changed(font->hb);
}

glyph_info * font_get_glyph(font_t * font, uint32_t glyphid, short pixel_size)
{
	glyph_cache * gcache = font_get_glyph_cache(font, pixel_size);
	assert(gcache);

	return glyph_cache_retrieve(gcache, glyphid);
}

glyph_cache * font_create_glyph_cache(font_t * font, short pixel_size)
{
	if (font->caches_fullness + 1 >= font->caches_capacity) {
		// resize the hash_table
		font->caches = (glyph_cache *)realloc(font->caches, 2 * font->caches_capacity * sizeof(glyph_cache));
		font->caches_capacity *= 2;

		log_info("font glyphcache list full, attempted realloc");
	}

	glyph_cache * gcache = &font->caches[font->caches_fullness];

	glyph_cache_create(gcache, font->face, 0, pixel_size);
	font->caches_fullness++;

	return gcache;
}

glyph_cache * font_get_glyph_cache(font_t * font, short pixel_size)
{
	glyph_cache * gcache = NULL;

	for (size_t i = 0; i < font->caches_fullness; i++) {
		if (font->caches[i].pixel_size == pixel_size) {
			gcache = &font->caches[i];
			break;
		}
	}

	if (gcache == NULL) {
		gcache = font_create_glyph_cache(font, pixel_size);
	}

	return gcache;
}

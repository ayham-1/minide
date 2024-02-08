#ifndef GLYPH_CACHE_H
#define GLYPH_CACHE_H

#include <GL/glew.h>

#include <ft2build.h>
#include FT_GLYPH_H

#include "logger.h"
#include "path.h"
#include "u8string.h"

#include "types/hash_table.h"

typedef struct {
	// https://learnopengl.com/img/in-practice/glyph.png
	float bearing_x;
	float bearing_y;

	float texture_x;
	float texture_y;

	FT_BitmapGlyph bglyph;
} glyph_info;

typedef struct {
	FT_Face font_face;
	size_t pixel_size;

	/* cache hash table */
	hash_table_t table; // uint32_t, glyph_info | font's glyphid, info
	size_t capacity;
	size_t fullness;
	uint32_t * keys;
	glyph_info * data;

	/* gpu atlas */
	GLuint atexID;
	GLuint atexOBJ;

	size_t awidth, aheight;
	size_t alast_offset_x, alast_offset_y;
	size_t alast_row_height;
} glyph_cache;

bool glyph_cache_create(glyph_cache * cache, FT_Face face, size_t capacity, size_t pixelSize);
void glyph_cache_cleanup(glyph_cache * cache);

glyph_info * glyph_cache_retrieve(glyph_cache * cache, uint32_t glyhid);
glyph_info * glyph_cache_append(glyph_cache * cache, uint32_t glyphid);

uint64_t __glyph_cache_table_hash(const uint8_t * const key);
bool __glyph_cache_table_entry_cleanup(hash_table_entry_t * entry);
bool __glyph_cache_table_eql_func(const uint8_t * const key1, const uint8_t * const key2);
void __glyph_cache_table_printer(const hash_table_entry_t * const entry);

void __glyph_cache_atlas_build(glyph_cache * cache);
void __glyph_cache_atlas_refill_gpu(glyph_cache * cache);
void __glyph_cache_atlas_append(glyph_cache * cache, glyph_info * info);

#define ATLAS_MAX_WIDTH 1024

#endif

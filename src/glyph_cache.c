#include "glyph_cache.h"

#include <assert.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <sys/mman.h>

static glyph_cache g_glyph_cache;

uint64_t hash(const uint8_t *const key) {
  // djb2
  // https://web.archive.org/web/20230906035458/http://www.cse.yorku.ca/~oz/hash.html
  uint64_t hash = 5381;
  hash = ((hash << 5) + hash) + *key;
  hash = ((hash << 5) + hash) + *(key + 1);
  hash = ((hash << 5) + hash) + *(key + 2);
  hash = ((hash << 5) + hash) + *(key + 3);
  return hash;
}

bool cleanup(hash_table_entry_t *entry) {
  // TODO(ayham): clean texture from GPU
  return true;
}

bool eql_func(const uint8_t *const key1, const uint8_t *const key2) {
  if (*key1 != *key2)
    return false;
  if (*(key1 + 1) != *(key2 + 1))
    return false;
  if (*(key1 + 2) != *(key2 + 2))
    return false;
  if (*(key1 + 3) != *(key2 + 3))
    return false;
  return true;
}

bool gc_init(path_t font, size_t capacity, size_t pixelSize,
             bool cacheEnglishTypeface) {
  if (FT_Init_FreeType(&g_glyph_cache.ft_library)) {
    log_error("failed library initialization.");
    return false;
  }

  if (FT_New_Face(g_glyph_cache.ft_library, (char *)font.fullPath.bytes, 0,
                  &g_glyph_cache.ft_face)) {
    log_error("failed font loading.");
    return false;
  }
  FT_Set_Pixel_Sizes(g_glyph_cache.ft_face, 0, pixelSize);

  log_info("loaded %i glyphs. YEEEPEE!",
           (int)g_glyph_cache.ft_face->num_glyphs);

  table_create((table_t *const)&g_glyph_cache.table, capacity,
                    hash, eql_func, cleanup);

  g_glyph_cache.table_keys = malloc(capacity * sizeof(wbyte_t));
  g_glyph_cache.table_data = malloc(capacity * sizeof(FT_Glyph));
  g_glyph_cache.last_key_offset = 0;
  g_glyph_cache.last_data_offset = 0;

  if (!cacheEnglishTypeface)
    return true;

  for (unsigned char i = 0; i <= 127; i++) {
    // TODO(ayham): more graceful error handling
    assert(gc_cache(i) != NULL);
  }

  return true;
}

void gc_cleanup() {
  // TODO(ayham): clean individual freetype glyphs
  FT_Done_Face(g_glyph_cache.ft_face);
  FT_Done_FreeType(g_glyph_cache.ft_library);

  hash_table_cleanup((hash_table_t *const)&g_glyph_cache.table);
  free(g_glyph_cache.table_keys);
  free(g_glyph_cache.table_data);
}

FT_Glyph *gc_cache(wbyte_t charID) {
  FT_Glyph *glyph =
      &g_glyph_cache.table_data[g_glyph_cache.last_data_offset++];
  g_glyph_cache.table_keys[g_glyph_cache.last_key_offset++] = charID;

  if (g_glyph_cache.last_data_offset > g_glyph_cache.table.capacity) {
    // TODO(ayham): realloc & copy to 2 * capacity
  }

  if (FT_Load_Glyph(g_glyph_cache.ft_face, charID, FT_LOAD_DEFAULT)) {
    log_error("unable to load glyph with charID %llx, %i", charID, charID);
    return NULL;
  }

  if (FT_Get_Glyph(g_glyph_cache.ft_face->glyph, glyph)) {
    log_error("unable to get glyph with charID %llx, %i", charID, charID);
    return NULL;
  }

  hash_table_insert(
      &g_glyph_cache.table,
      (void *)&g_glyph_cache.table_keys[g_glyph_cache.last_key_offset - 1],
      (void *)&g_glyph_cache
          .table_data[g_glyph_cache.last_data_offset - 1]);

  // TODO(ayham): load into GPU
  return glyph;
}

FT_Glyph *gc_get_by_index(wbyte_t charID);

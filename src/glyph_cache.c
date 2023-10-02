#include "glyph_cache.h"

#include <assert.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <sys/mman.h>

bool glyph_cache_init(glyph_cache* cache, 
                      GLuint textureID,
                      path_t font, 
                      size_t capacity, 
                      size_t pixelSize,
                      bool cacheEnglishTypeface) {
    assert(cacheEnglishTypeface || capacity > 0);
    cache->atexID = textureID;

    if (FT_Init_FreeType(&cache->ft_library)) {
        log_error("failed library initialization.");
        return false;
    }

    if (FT_New_Face(cache->ft_library, (char *)font.fullPath.bytes, 0,
                    &cache->ft_face)) {
        log_error("failed font loading.");
        return false;
    }
    FT_Set_Pixel_Sizes(cache->ft_face, 0, pixelSize);

    log_info("font contains %i glyphs. YEEEPEE!",
             (int)cache->ft_face->num_glyphs);

    if (cacheEnglishTypeface) {
        cache->capacity = 96;
    } else {
        cache->capacity = capacity;
    }

    hash_table_create((hash_table_t *const)&cache->table,
                      capacity,
                      __glyph_cache_table_hash,
                      __glyph_cache_table_eql_func,
                      __glyph_cache_table_entry_cleanup);

    cache->keys = malloc(cache->capacity * sizeof(u8encode));
    cache->data = malloc(cache->capacity * sizeof(glyph_info));
    cache->fullness = 0;

    if (cacheEnglishTypeface)
        for (unsigned char i = 32; i <= 127; i++) {
            // TODO(ayham): more graceful error handling
            assert(glyph_cache_append(cache, i) != NULL);
        }

    __glyph_cache_atlas_build(cache);
    __glyph_cache_atlas_refill_gpu(cache);

    return true;
}

void glyph_cache_cleanup(glyph_cache* cache) {
    FT_Done_Face(cache->ft_face);
    FT_Done_FreeType(cache->ft_library);

    hash_table_cleanup((hash_table_t *const)&cache->table);
    free(cache->keys);
    free(cache->data);
}

glyph_info* glyph_cache_retrieve(glyph_cache* cache, 
                                 uint32_t glyphid) {
    hash_table_entry_t* entry = NULL;

    glyph_info* info = NULL;
    log_info("retrieving glyphid %i", glyphid);

    if (!hash_table_get(&cache->table, 
                   (uint8_t*) &glyphid,
                   &entry)) {
        info = glyph_cache_append(cache, glyphid);
        log_warn("glyphid was not in cache %li", glyphid);
    }
    else {
        info = (glyph_info*) entry->data;
    }

    if (info == NULL) {
        log_error("unable to cache glyphid %li", glyphid);
    }

    return info;
}

glyph_info* glyph_cache_append(glyph_cache* cache, 
                               uint32_t glyphid) {
    if (cache->fullness >= cache->table.capacity) {
        cache->keys = realloc(cache->keys, 2 * cache->table.capacity * sizeof(u8encode));
        cache->data = realloc(cache->data, 2 * cache->table.capacity * sizeof(glyph_info));
        cache->table.capacity *= 2;
    }

    glyph_info* info = &cache->data[cache->fullness];
    cache->keys[cache->fullness] = glyphid;

    if (FT_Load_Glyph(cache->ft_face, cache->keys[cache->fullness], FT_LOAD_RENDER)) {
        log_error("unable to load glyph with glyphid %li", glyphid);
        return NULL;
    }

    FT_Glyph tglyph;
    if (FT_Get_Glyph(cache->ft_face->glyph, &tglyph)) {
        log_error("unable to get glyph with glyphid %li", glyphid);
        return NULL;
    }

    // ensure bitmap is present
    if ((tglyph->format != FT_GLYPH_FORMAT_BITMAP)
        && FT_Glyph_To_Bitmap(&tglyph, FT_RENDER_MODE_NORMAL, 0, 1)) {
        log_error("unable to get glyph's bitmap of glyphid %li", glyphid);
    }

    info->bglyph = (FT_BitmapGlyph) tglyph;

    info->bearing_x = cache->ft_face->glyph->bitmap_left;
    info->bearing_y = cache->ft_face->glyph->bitmap_top;

    hash_table_insert(
        &cache->table,
        (void *)&cache->keys[cache->fullness],
        (void *)&cache->data[cache->fullness]);
    cache->fullness++;

//    __glyph_cache_atlas_append(cache, info);

    return info;
}

void __glyph_cache_atlas_build(glyph_cache* cache) {
    unsigned int row_width = 0, row_height = 0;
    cache->awidth = 0;
    cache->aheight = 0;

    for (size_t i = 0; i < cache->fullness; i++) {
        glyph_info* info = &cache->data[i];

        if (row_width + info->bglyph->bitmap.width > ATLAS_MAX_WIDTH) {
            if (cache->awidth < row_width) cache->awidth = row_width;
            cache->aheight += row_height;
            row_width = 0;
            row_height = 0;
        }

        if (info->bglyph->bitmap.rows > row_height) row_height = info->bglyph->bitmap.rows;
        row_width += info->bglyph->bitmap.width;
    }

    if (cache->awidth < row_width) cache->awidth = row_width;
    cache->aheight += row_height;

    glActiveTexture(cache->atexID);
    glGenTextures(1, &cache->atexOBJ);
    glBindTexture(GL_TEXTURE_2D, cache->atexOBJ);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 
                 cache->awidth, cache->aheight, 
                 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void __glyph_cache_atlas_refill_gpu(glyph_cache* cache) {
    /* assumes that the texture on the gpu is the appropriate size */
    unsigned int offset_x = 0, offset_y = 0;
    unsigned int row_width = 0, row_height = 0;

    for (size_t i = 0; i < cache->capacity; i++) {
        glyph_info* info = &cache->data[i];
        if (info->bglyph == NULL) {
            log_warn("refilling with NULL bglyph, charcode 0x%21x", cache->keys[i]);
            continue;
        }
        log_info("0x%21x", cache->keys[i]);

        if (offset_x + info->bglyph->bitmap.width > ATLAS_MAX_WIDTH) {
            offset_y += row_height;
            row_height = 0;
            offset_x = 0;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, 
                        offset_x, offset_y,
                        info->bglyph->bitmap.width, info->bglyph->bitmap.rows,
                        GL_RED,
                        GL_UNSIGNED_BYTE,
                        info->bglyph->bitmap.buffer);

        info->texture_x = offset_x / (float) cache->awidth;
        info->texture_y = offset_y / (float) cache->aheight;

        if (row_height < info->bglyph->bitmap.rows) row_height = info->bglyph->bitmap.rows;
        offset_x += info->bglyph->bitmap.width;
    }

    cache->alast_offset_x = offset_x;
    cache->alast_offset_y = offset_y;
    cache->alast_row_height = row_height;
}

void __glyph_cache_atlas_append(glyph_cache* cache, 
                                glyph_info* info) {
    // TODO(ayham): research if resizing texture can be done without refilling it.
    if (cache->alast_row_height < info->bglyph->bitmap.rows) {
        __glyph_cache_atlas_refill_gpu(cache);
        return;
    }

    if (cache->alast_offset_x + info->bglyph->bitmap.width > ATLAS_MAX_WIDTH) {
        __glyph_cache_atlas_refill_gpu(cache);
        return;
    }

    glActiveTexture(cache->atexID);
    glBindTexture(GL_TEXTURE_2D, cache->atexOBJ);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 
                    cache->alast_offset_x, cache->alast_offset_y,
                    info->bglyph->bitmap.width, info->bglyph->bitmap.rows,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    info->bglyph->bitmap.buffer);

    cache->alast_offset_x += info->bglyph->bitmap.width;
    cache->alast_offset_y += info->bglyph->bitmap.rows;
}

uint64_t __glyph_cache_table_hash(const uint8_t *const key) {
    // djb2
    // https://web.archive.org/web/20230906035458/http://www.cse.yorku.ca/~oz/hash.html
    uint64_t hash = 5381;
    hash = ((hash << 5) + hash) + *key;
    hash = ((hash << 5) + hash) + *(key + 1);
    hash = ((hash << 5) + hash) + *(key + 2);
    hash = ((hash << 5) + hash) + *(key + 3);
    return hash;
}

bool __glyph_cache_table_entry_cleanup(hash_table_entry_t *entry) {
    // we basically refer to `table_keys` and `table_data` for hash table entries,
    // those are cleaned when `gc_cleanup` is called
    return true;
}

bool __glyph_cache_table_eql_func(const uint8_t *const key1, const uint8_t *const key2) {
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

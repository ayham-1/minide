#include "glyph_cache.h"

#include "aristotle.h"
#include "texture_lender.h"

#include <assert.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <sys/mman.h>

#define KEY_TYPE uint32_t
#define DATA_TYPE glyph_info

bool glyph_cache_init(glyph_cache* cache, 
                      path_t font, 
                      size_t capacity, 
                      size_t pixelSize) {

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

    if (cache->capacity < 96) {
        cache->capacity = 96;
    } else {
        cache->capacity = capacity;
    }

    hash_table_create((hash_table_t *const)&cache->table,
                      capacity,
                      __glyph_cache_table_hash,
                      __glyph_cache_table_eql_func,
                      __glyph_cache_table_entry_cleanup);

    cache->keys = malloc(cache->capacity * sizeof(KEY_TYPE));
    cache->data = malloc(cache->capacity * sizeof(DATA_TYPE));
    cache->fullness = 0;

    DATA_TYPE* notdef = glyph_cache_append(cache, 0);
    assert(notdef);

    for (unsigned char i = 32; i < 127; i++) {
        FT_ULong id = FT_Get_Char_Index(cache->ft_face, i);
        DATA_TYPE* res = glyph_cache_append(cache, id);
        assert(res);
        assert(res->bglyph);
        (void)res;
    }

    __glyph_cache_atlas_build(cache);
    __glyph_cache_atlas_refill_gpu(cache);

    cache->atexID = texture_lender_request();
    log_info("created new glyph_cache, atexID: %d", cache->atexID);

    //hash_table_debug(&cache->table, __glyph_cache_table_printer);
    
    return true;
}

void glyph_cache_cleanup(glyph_cache* cache) {
    FT_Done_Face(cache->ft_face);
    FT_Done_FreeType(cache->ft_library);

    hash_table_cleanup((hash_table_t *const)&cache->table);
    free(cache->keys);
    free(cache->data);

    texture_lender_return(cache->atexID);
}

DATA_TYPE* glyph_cache_retrieve(glyph_cache* cache, 
                                 KEY_TYPE glyphid) {
    hash_table_entry_t* entry = NULL;
    DATA_TYPE* info = NULL;

    if (!hash_table_get(&cache->table, 
                   (uint8_t*) &glyphid,
                   &entry)) {
        info = glyph_cache_append(cache, glyphid);
        log_warn("glyphid was not in cache %li, attempted to cache", glyphid);
        __glyph_cache_atlas_append(cache, info);
    }
    else {
        info = (DATA_TYPE*) entry->data;
    }

    if (info == NULL) {
        DATA_TYPE* info = glyph_cache_retrieve(cache, 0);
        assert(info);
        log_error("unable to cache glyphid %li", glyphid);
        log_warn("returning .notdef glyph");
        return info;
    }

    assert(info->bglyph);
    return info;
}

DATA_TYPE* glyph_cache_append(glyph_cache* cache, 
                               KEY_TYPE glyphid) {
    if (cache->fullness + 1 >= cache->capacity) {
        // resize the hash_table
        cache->keys = (KEY_TYPE*) realloc(cache->keys, 2 * cache->capacity * sizeof(KEY_TYPE));
        cache->data = (DATA_TYPE*) realloc(cache->data, 2 * cache->capacity * sizeof(DATA_TYPE));
        cache->capacity *= 2;

        hash_table_cleanup(&cache->table);
        hash_table_create((hash_table_t *const)&cache->table,
                      cache->capacity,
                      __glyph_cache_table_hash,
                      __glyph_cache_table_eql_func,
                      __glyph_cache_table_entry_cleanup);

        for (size_t i = 0; i < cache->fullness; i++) {
            hash_table_insert(&cache->table, (void*) &cache->keys[i], (void*) &cache->data[i]);
        }

        log_info("cache full, attempted realloc");
    }
    DATA_TYPE* info = &cache->data[cache->fullness];
    cache->keys[cache->fullness] = glyphid;

    if (FT_Load_Glyph(cache->ft_face, cache->keys[cache->fullness], FT_LOAD_DEFAULT | FT_LOAD_RENDER)) {
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

    assert(info);
    info->bglyph = (FT_BitmapGlyph) tglyph;
    assert(tglyph);
    assert(info->bglyph);

    info->bearing_x = cache->ft_face->glyph->bitmap_left;
    info->bearing_y = cache->ft_face->glyph->bitmap_top;

    hash_table_insert(
        &cache->table,
        (void *) &cache->keys[cache->fullness],
        (void *) &cache->data[cache->fullness]);
    cache->fullness++;

    return info;
}

void __glyph_cache_atlas_build(glyph_cache* cache) {
    unsigned int row_width = 0, row_height = 0;
    cache->awidth = ATLAS_MAX_WIDTH;
    cache->aheight = 0;

    for (size_t i = 0; i < cache->table.capacity; i++) {
        if (cache->table.buckets[i] == NULL) continue;

        hash_table_entry_t* curr_entry = cache->table.buckets[i];

        while (curr_entry != NULL) {
            DATA_TYPE* info = (DATA_TYPE*) (*curr_entry).data;

            if (row_width + info->bglyph->bitmap.width > ATLAS_MAX_WIDTH) {
                cache->aheight += row_height;
                row_width = 0;
                row_height = 0;
            }

            if (info->bglyph->bitmap.rows > row_height) row_height = info->bglyph->bitmap.rows;
            row_width += info->bglyph->bitmap.width;
            curr_entry = curr_entry->next;
        }
    }

    if (cache->awidth < row_width) cache->awidth = row_width;
    cache->aheight += row_height;

    glActiveTexture(GL_TEXTURE0);
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
    /* assumes that the texture from freetype is the appropriate size */
    GLfloat offset_x = 0, offset_y = 0;
    unsigned int row_height = 0;

    for (size_t i = 0; i < cache->table.capacity; i++) {
        if (cache->table.buckets[i] == NULL) continue;
      
        hash_table_entry_t* curr_entry = cache->table.buckets[i];

        while (curr_entry != NULL) {
            DATA_TYPE* info = (DATA_TYPE*) (*curr_entry).data;
            KEY_TYPE* key = (KEY_TYPE*) (*curr_entry).key;

            if (info->bglyph == NULL) {
                log_warn("refilling with NULL bglyph, charcode 0x%21x", key);
                continue;
            }
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

            if (row_height < info->bglyph->bitmap.rows)
                row_height = info->bglyph->bitmap.rows;
            offset_x += info->bglyph->bitmap.width;

            curr_entry = curr_entry->next;
        }
    }

    cache->alast_offset_x = offset_x;
    cache->alast_offset_y = offset_y;
    cache->alast_row_height = row_height;
}

void __glyph_cache_atlas_append(glyph_cache* cache, 
                                DATA_TYPE* info) {
    if (cache->alast_row_height < info->bglyph->bitmap.rows) {
        __glyph_cache_atlas_build(cache);
        __glyph_cache_atlas_refill_gpu(cache);
        return;
    }

    if (cache->alast_offset_x + info->bglyph->bitmap.width >= ATLAS_MAX_WIDTH) {
        __glyph_cache_atlas_build(cache);
        __glyph_cache_atlas_refill_gpu(cache);
        return;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cache->atexOBJ);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 
                    cache->alast_offset_x, cache->alast_offset_y,
                    info->bglyph->bitmap.width, info->bglyph->bitmap.rows,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    info->bglyph->bitmap.buffer);

    info->texture_x = cache->alast_offset_x / (float) cache->awidth;
    info->texture_y = cache->alast_offset_y / (float) cache->aheight;

    cache->alast_offset_x += info->bglyph->bitmap.width;
    //cache->alast_offset_y += info->bglyph->bitmap.rows;
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
    (void)entry;
    return true;
}

bool __glyph_cache_table_eql_func(const uint8_t *const key1, const uint8_t *const key2) {
    KEY_TYPE key_1 = *(KEY_TYPE*) key1;
    KEY_TYPE key_2 = *(KEY_TYPE*) key2;
    return key_1 == key_2;
}

void __glyph_cache_table_printer(const hash_table_entry_t* const entry) {
    KEY_TYPE glyphid = *(KEY_TYPE*) entry->key;
    DATA_TYPE info = *(DATA_TYPE*) entry->data;

    log_debug("\tfont's glyphid: %li", glyphid);
    log_debug("\t glyph_info:");

    log_debug("\t\t info.bearing_x: %f", info.bearing_x);
    log_debug("\t\t info.bearing_y: %f", info.bearing_y);
    log_debug("\t\t info.texture_x: %f", info.texture_x);
    log_debug("\t\t info.texture_y: %f", info.texture_y);

    log_debug("\t\t info.bglyph->bitmap.width: %i", info.bglyph->bitmap.width);
    log_debug("\t\t info.bglyph->bitmap.rows: %i", info.bglyph->bitmap.rows);
}

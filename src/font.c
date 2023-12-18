#include "font.h"

#include <limits.h>
#include <assert.h>

#define KEY_TYPE short
#define DATA_TYPE glyph_cache

font_t* font_create(FT_Library ft_lib,
                  fc_holder* fc_holder) {
    font_t* result = malloc(sizeof(font_t));

    result->fc_holder = fc_holder;

    if (FT_New_Face(ft_lib, fc_get_path_by_font(fc_holder), 0, &result->face)) {
        log_error("failed font creation");
        return NULL;
    }

    result->hb = hb_ft_font_create_referenced(result->face);
    //FT_Set_Pixel_Sizes(result->face, 0, 18);

    result->table_capacity = 3;
    result->table_fullness = 0;
    hash_table_create((hash_table_t *const)&result->table,
                      result->table_capacity,
                      __font_table_hash,
                      __font_table_eql_func,
                      __font_table_entry_cleanup);

    result->table_keys = calloc(result->table_capacity,
                                sizeof(KEY_TYPE));
    result->table_data = calloc(result->table_capacity,
                                sizeof(DATA_TYPE));

    return result;
}

void font_clean(font_t* font) {
    hb_font_destroy(font->hb);
    FT_Done_Face(font->face);

    free(font);
}

bool font_does_have_charid(font_t* font, uint32_t charid) {
    return FT_Get_Char_Index(font->face, charid);
}

void font_set_pixel_size(font_t* font, short pixel_size) {
    /*
     * This function changes the size of FT_Face and hb_font_t,
     * it is done centrally here as a way to connect harfbuzz and freetype,
     * font_t holds glyph_cache per pixel_size, however, the switch between them is not 
     * done explicitly in glyph_cache but rather here. And the configuration of correct size is
     * done here by FT_Set_Pixel_Sizes() and hb_font_changed()
     *
     * To correctly use this function, call before doing hb_shape() - shaper.h does this.
     */
    if (FT_IS_SCALABLE(font->face)) {
        FT_Set_Pixel_Sizes(font->face, 0, pixel_size);
        font->scale = 1.0f;
    } else {
        // pick closest in font->face->available_sizes[]
        short distance = SHRT_MAX;
        int index = 0; 
        for (FT_Int i = 0; i < font->face->num_fixed_sizes; i++) {
            short new_distance = abs(font->face->available_sizes[i].height - (FT_Short) pixel_size);
            if (distance >= new_distance) {
                distance = new_distance;
                index = i;
            }
        }

        FT_Select_Size(font->face, index);
        font->scale = ((float) pixel_size) / ((float) font->face->available_sizes[index].height);
        log_warn("using manual font scaling");
    }

    hb_font_changed(font->hb);
}

glyph_info* font_get_glyph(font_t* font, uint32_t glyphid, short pixel_size) {
    glyph_cache* gcache = font_get_glyph_cache(font, pixel_size);
    assert(gcache);

    return glyph_cache_retrieve(gcache, glyphid);
}

glyph_cache* font_create_glyph_cache(font_t* font, short pixel_size) {
    if (font->table_fullness + 1 >= font->table_capacity) {
        // resize the hash_table
        font->table_keys = (KEY_TYPE*) realloc(font->table_keys, 2 * font->table_capacity * sizeof(KEY_TYPE));
        font->table_data = (DATA_TYPE*) realloc(font->table_data, 2 * font->table_capacity * sizeof(DATA_TYPE));
        font->table_capacity *= 2;

        hash_table_cleanup(&font->table);
        hash_table_create((hash_table_t *const)&font->table,
                      font->table_capacity,
                      __font_table_hash,
                      __font_table_eql_func,
                      __font_table_entry_cleanup);

        for (size_t i = 0; i < font->table_fullness; i++) {
            hash_table_insert(&font->table, 
                              (void*) &font->table_keys[i], 
                              (void*) &font->table_data[i]);
        }

        log_info("font table full, attempted realloc");
    }

    DATA_TYPE* gcache = &font->table_data[font->table_fullness];
    font->table_keys[font->table_fullness] = pixel_size;

    glyph_cache_create(gcache, font->face, 0, pixel_size);

    hash_table_insert(&font->table,
                      (void*) &font->table_keys[font->table_fullness],
                      (void*) &font->table_data[font->table_fullness]);
    font->table_fullness++;

    return gcache;
}

glyph_cache* font_get_glyph_cache(font_t* font, short pixel_size) {
    hash_table_entry_t* entry = NULL;
    glyph_cache* gcache = NULL;

    if (!hash_table_get(&font->table,
                        (uint8_t*) &pixel_size,
                        &entry)) {
        gcache = font_create_glyph_cache(font, pixel_size);
    } else 
        gcache = (DATA_TYPE*) entry->data;

    if (gcache == NULL) {
        log_error("font table reported available gcache, however value is NULL");

        gcache = font_create_glyph_cache(font, pixel_size);
    }

    return gcache;
}

uint64_t __font_table_hash(const uint8_t *const key) {
    // djb2
    // https://web.archive.org/web/20230906035458/http://www.cse.yorku.ca/~oz/hash.html
    uint64_t hash = 5381;
    hash = ((hash << 5) + hash) + *key;
    hash = ((hash << 5) + hash) + *(key + 1);
    hash = ((hash << 5) + hash) + *(key + 2);
    hash = ((hash << 5) + hash) + *(key + 3);
    return hash;
}


bool __font_table_entry_cleanup(hash_table_entry_t *entry) {
    (void)entry;
    return true;
}

bool __font_table_eql_func(const uint8_t *const key1, const uint8_t *const key2) {
    KEY_TYPE key_1 = *(KEY_TYPE*) key1;
    KEY_TYPE key_2 = *(KEY_TYPE*) key2;
    return key_1 == key_2;
}

void __font_table_printer(const hash_table_entry_t* const entry) {
    KEY_TYPE size = *(KEY_TYPE*) entry->key;

    log_debug("\tfont's size: %li", size);
}

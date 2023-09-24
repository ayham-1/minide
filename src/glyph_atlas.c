#include "glyph_atlas.h"

#include <math.h>

uint64_t __table_hash(const uint8_t* const key) {
    // djb2
    // https://web.archive.org/web/20230906035458/http://www.cse.yorku.ca/~oz/hash.html
    uint64_t hash = 5381;
    hash = ((hash << 5) + hash) + *key;
    hash = ((hash << 5) + hash) + *(key + 1);
    hash = ((hash << 5) + hash) + *(key + 2);
    hash = ((hash << 5) + hash) + *(key + 3);
    return hash;
}

bool __table_cleanup(hash_table_entry_t* entry) {
    return true;
}

bool __table_eql_func(const uint8_t* const key1, 
                      const uint8_t* const key2) {
    if (*key1 != *key2) return false;
    if (*(key1 + 1) != *(key2 + 1)) return false;
    if (*(key1 + 2) != *(key2 + 2)) return false;
    if (*(key1 + 3) != *(key2 + 3)) return false;
    return true;
}

bool glyph_gpu_atlas_build(glyph_gpu_atlas* const atlas,
                           GLuint texID,
                           size_t initial_capacity,
                           u8encode* encodings,
                           FT_Glyph* glyphs) {
    atlas->textureID = texID;
    atlas->capacity = initial_capacity;
    atlas->fullness = initial_capacity;
    atlas->glyphs = malloc(sizeof(glyph_info) * atlas->capacity);
    memset(atlas->glyphs, 0, sizeof(glyph_info) * atlas->capacity);

    hash_table_create(atlas->table,
                      atlas->capacity,
                      __table_hash,
                      __table_eql_func,
                      __table_cleanup);

    unsigned int row_width = 0;
    unsigned int row_height = 0;
    atlas->width = 0;
    atlas->height = 0;

    for (size_t i = 0; i < atlas->capacity; i++) {
        FT_Glyph glyph = glyphs[i];
        if ((glyph->format != FT_GLYPH_FORMAT_BITMAP)
            && FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1)) {
            log_error("unable to get glyph's bitmap of charcode 0x%21X", encodings[i]);
            continue;
        }
        FT_BitmapGlyph bglyph = (FT_BitmapGlyph)glyph;
        
        if (row_width + bglyph->bitmap.width >= ATLAS_MAX_WIDTH) {
            atlas->width = atlas->width >= row_width ? atlas->width : row_width;
            atlas->height += row_height;
            row_width = 0;
            row_height = 0;
        }

        row_height = row_height >= bglyph->bitmap.rows ? 
            row_height : bglyph->bitmap.rows;

        row_width += bglyph->bitmap.width;
    }
    atlas->width = atlas->width >= row_width ? atlas->width : row_width;
    atlas->height += row_height;

    glActiveTexture(atlas->textureID);
    glGenTextures(1, &atlas->textureObj);
    glBindTexture(GL_TEXTURE_2D, atlas->textureObj);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas->width, atlas->height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned int offset_x = 0;
    unsigned int offset_y = 0;
    row_width = 0;
    row_height = 0;

    for (size_t i = 0; i < atlas->capacity; i++) {
        FT_BitmapGlyph bglyph = (FT_BitmapGlyph)glyphs[i];

        if (offset_x + bglyph->bitmap.width >= ATLAS_MAX_WIDTH) {
            offset_y += row_height;
            row_height = 0;
            offset_x = 0;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, 
                        offset_x, offset_y,
                        bglyph->bitmap.width, bglyph->bitmap.rows,
                        GL_RED,
                        GL_UNSIGNED_BYTE,
                        bglyph->bitmap.buffer);
        
        glyph_info* info = &atlas->glyphs[i];
        info->tx = offset_x / (float) atlas->width;
        info->ty = offset_y / (float) atlas->height;

        hash_table_insert(atlas->table,
                          (uint8_t* const) &encodings[i],
                          (uint8_t* const) info);

        row_height = row_height >= bglyph->bitmap.rows ?
                    row_height : bglyph->bitmap.rows;
        offset_x += bglyph->bitmap.width;
    }

    atlas->last_offset_x = offset_x;
    atlas->last_offset_y = offset_y;

    log_info("generated glyph atlas of dimenions %dx%d (%d kb)",
             atlas->width, atlas->height,
             (atlas->width * atlas->height) /  1024);
    log_info("glIsTexture(%d) == %d", atlas->textureID, glIsTexture(atlas->textureID));

    return true;
}

/* TODO(ayham): used with text renderer and written when that is written */
bool glyph_gpu_atlas_activate(glyph_gpu_atlas* const atlas,
                              u8encode* encoding);

bool glyph_gpu_atlas_append(glyph_gpu_atlas* const atlas,
                            u8encode* encoding,
                            FT_Glyph glyph) {
    
    hash_table_entry_t* temp_info = NULL;
    if (hash_table_get(atlas->table, (uint8_t*) encoding, &temp_info)) return true;

    if ((glyph->format != FT_GLYPH_FORMAT_BITMAP)
        && FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1)) {
        log_error("unable to get glyph's bitmap of charcode 0x%21X", encoding);
    }

    if (atlas->fullness >= atlas->capacity) {
        atlas->glyphs = realloc(atlas->glyphs,
                                2 * atlas->capacity * sizeof(glyph_info));
        atlas->capacity *= 2;
    }

    FT_BitmapGlyph bglyph = (FT_BitmapGlyph)glyph;
    atlas->last_row_height = atlas->last_row_height >= bglyph->bitmap.rows ?
                    atlas->last_row_height : bglyph->bitmap.rows;

    if (atlas->last_offset_x + bglyph->bitmap.width >= ATLAS_MAX_WIDTH) {
        atlas->last_offset_y += atlas->last_row_height;
        atlas->last_row_height = 0;
        atlas->last_offset_x = 0;
    }

    glyph_info* info = malloc(sizeof(glyph_info));
    info->tx = atlas->last_offset_x / (float) atlas->width;
    info->ty = atlas->last_offset_y / (float) atlas->height;

    hash_table_insert(atlas->table,
                          (uint8_t* const) encoding,
                          (uint8_t* const) info);

    glBindTexture(GL_TEXTURE_2D, atlas->textureObj);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 
                    atlas->last_offset_x, atlas->last_offset_y,
                    bglyph->bitmap.width, bglyph->bitmap.rows,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    bglyph->bitmap.buffer);

    return true;
}

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>

#include "../src/types/hash_table.h"
#include "../src/logger.h"
#include "../src/u8string.h"
#include "../src/path.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

uint64_t hash(uint8_t* key) {
    // djb2
    // https://web.archive.org/web/20230906035458/http://www.cse.yorku.ca/~oz/hash.html
    uint64_t hash = 5381;
    hash = ((hash << 5) + hash) + *key;
    hash = ((hash << 5) + hash) + *(key + 1);
    hash = ((hash << 5) + hash) + *(key + 2);
    hash = ((hash << 5) + hash) + *(key + 3);
    return hash;
}

bool cleanup(hash_table_entry_t* entry) {
    free(entry->key);
    return true;
}

int main(int argc, char *argv[]) {
    logger_init(DEBUG, "", false);
    FT_Library lib;
    if (FT_Init_FreeType(&lib)) {
        log_error("failed library initialization.");
        return false;
    }

    FT_Face face;
    if (FT_New_Face(lib, "assets/FreeSans.ttf", 0, &face)) {
        log_error("failed font loading.");
        return false;
    }
    FT_Set_Pixel_Sizes(face, 0, 12);

    log_info("loaded %i glyphs.", 
             (int)face->num_glyphs);

    FT_Select_Charmap(face, FT_ENCODING_UNICODE);

    hash_table_t table;
    hash_table_create(&table, 2 * face->num_glyphs, 0, false, hash, cleanup);
    
    FT_ULong charcode;
    FT_UInt  gid;

    setlocale(LC_ALL, "");
    charcode = FT_Get_First_Char(face, &gid);
    while (gid != 0) {
        log_debug("codepoint: %llu gid: %u", (unsigned long long)charcode, gid);
        charcode = FT_Get_Next_Char(face, charcode, &gid);
        FT_ULong* c = malloc(sizeof(FT_ULong));
        *c = gid;
        hash_table_insert(&table, (uint8_t*)c, (uint8_t*)c);
    }

    log_info("collisions: %i", table.collisions);
    log_info("charmaps: %i", face->num_charmaps);
    log_info("glyphs: %i", face->num_glyphs);
    log_info("fixed sizes: %i", face->num_fixed_sizes);

    hash_table_cleanup(&table);
    logger_cleanup();
    FT_Done_Face(face);
    FT_Done_FreeType(lib);
    return 0;
}

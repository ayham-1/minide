#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../src/types/hash_table.h"
#include "../src/logger.h"
#include "../src/u8string.h"
#include "../src/path.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#define FNV_prime 0x100000001b3

uint64_t hash(uint8_t* key) {
    /* FNV 1a for UTF-8 */
    uint64_t hash = 0xcbf29ce484222325;
    
    hash ^= *key;
    hash ^= FNV_prime;

    hash ^= *(key+1);
    hash ^= FNV_prime;

    hash ^= *(key+2);
    hash ^= FNV_prime;

    hash ^= *(key+3);
    hash ^= FNV_prime;

    return 1;
}

bool cleanup(hash_table_entry_t* entry) {
    return true;
}

int main(int argc, char *argv[]) {
    logger_init(DEBUG, "", false);

    hash_table_t table;
    hash_table_create(&table, 100, 0, false, hash, cleanup);

    log_info("hash collisions: %i", table.collisions);

    FT_Library lib;
    if (FT_Init_FreeType(&lib)) {
        log_error("failed library initialization.");
        return false;
    }

    FT_Face face;
    if (FT_New_Face(lib, "assets/unifont.ttf", 0, &face)) {
        log_error("failed font loading.");
        return false;
    }
    FT_Set_Pixel_Sizes(face, 0, 12);

    log_info("loaded %i glyphs.", 
             (int)face->num_glyphs);

    hash_table_cleanup(&table);
    logger_cleanup();
    //FT_Done_Face(face);
    FT_Done_FreeType(lib);
    return 0;
}

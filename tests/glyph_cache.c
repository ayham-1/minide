#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../src/path.h"
#include "../src/logger.h"
#include "../src/glyph_cache.h"

#define PATH "assets/FreeSans.ttf"
#define PATH_BYTES_NUM sizeof(PATH)

int main(int argc, char *argv[]) {
    logger_init(DEBUG, "", false);

    path_t p;
    path_create(&p, PATH_BYTES_NUM);
    memcpy(p.fullPath.bytes, PATH, PATH_BYTES_NUM);

    gc_init(p, 512, 24, true);

    path_cleanup(&p);
    gc_cleanup();
    logger_cleanup();
    return 0;
}

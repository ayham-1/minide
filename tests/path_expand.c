#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../src/path.h"
#include "../src/logger.h"

#define path "test.txt"
#define path_BYTES_NUM sizeof(path)

int main(int argc, char *argv[]) {
    logger_init(DEBUG, "", false);

    path_t p = {0};
    path_create(&p, path_BYTES_NUM);
    memcpy(path, p.fullPath->bytes, path_BYTES_NUM);

    path_expand(&p);

    path_cleanup(&p);
    logger_cleanup();
    return 0;
}

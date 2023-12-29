#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/u8string.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
    assert(u8str_from_code_point((byte_t *)"U+4E3E\0\0") ==
           0b11100100101110001011111000000000);
    return 0;
}

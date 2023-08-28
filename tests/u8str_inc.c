#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../src/u8string.h"

#define HELLO "HELLO 👋 WORLD 🌏"
#define HELLO_BYTES_NUM 20
#define HELLO_LEN 15

int main(int argc, char *argv[]) {
    string_t* ustr = u8str_create(HELLO_BYTES_NUM);

    memcpy(ustr->bytes, HELLO, HELLO_BYTES_NUM);

    size_t incs = 0;

    byte_t* ptr = ustr->bytes;
    while (u8str_inc(ustr, &ptr)) incs++;
    printf("%i", (int)incs);

    // '- 1' -> must not increment past last character
    assert(incs == HELLO_LEN - 1);
    assert(*ptr == 0b11110000);

    return 0;
}

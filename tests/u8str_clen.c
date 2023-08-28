#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../src/u8string.h"

#define HELLO "HELLO 👋 WORLD 🌏"
#define HELLO_BYTES_NUM 21
#define HELLO_LEN 16 // because C adds \0, not needed

int main(int argc, char *argv[]) {
    string_t* ustr = u8str_create(HELLO_BYTES_NUM);

    assert(u8str_get_bytes_needed_for(MAKE_BYTE_WBYTE(0xF0)) == 4);

    memcpy(ustr->bytes, HELLO, HELLO_BYTES_NUM);

    assert(u8str_clen(ustr) == HELLO_LEN);

    return 0;
}

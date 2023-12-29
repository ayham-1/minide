#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/u8string.h"

#define HELLO "HELLO 👋 WORLD 🌏"
#define HELLO_BYTES_NUM 21
#define HELLO_LEN 15

int main(int argc, char *argv[]) {
    string_t ustr;
    u8str_create(&ustr, HELLO_BYTES_NUM);
    memcpy(ustr.bytes, HELLO, HELLO_BYTES_NUM);

    assert(u8str_get_bytes_needed_for(MAKE_BYTE_WBYTE(0xF0)) == 4);

    assert(u8str_clen(&ustr) == HELLO_LEN);

    u8str_cleanup(&ustr);
    return 0;
}

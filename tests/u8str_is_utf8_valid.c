#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../src/u8string.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
    assert(u8str_is_utf8_valid('a'));           // single-byte
    assert(u8str_is_utf8_valid(0xC280));        // two-byte
    assert(u8str_is_utf8_valid(0xE182A6));      // three-byte
    assert(u8str_is_utf8_valid(0xF09FA880));    // four-byte

    assert(!u8str_is_utf8_valid(0x2FC0AE2E));
    
    return 0;
}

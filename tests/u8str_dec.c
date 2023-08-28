#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../src/u8string.h"

#define HELLO "HELLO 👋 WORLD 🌏"
#define HELLO_BYTES_NUM sizeof(HELLO) - 1 // ignore \0
#define HELLO_LEN 15

int main(int argc, char *argv[]) {
    string_t* ustr = u8str_create(HELLO_BYTES_NUM);

    memcpy(ustr->bytes, HELLO, HELLO_BYTES_NUM);

    size_t decs = 0; // haha, funny name, haha

    byte_t* ptr = ustr->bytes + ustr->size - 1;
    while (u8str_dec(ustr, &ptr)) decs++;
    fprintf(stdout,"%i", (int)decs);

    // -1 we do not decrement past first character
    assert(decs == HELLO_LEN - 1);
    assert(*ptr == 'H');

    return 0;
}

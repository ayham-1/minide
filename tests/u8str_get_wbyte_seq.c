#include <assert.h>

#include "../src/u8string.h"

int main(int argc, char *argv[]) {
    byte_t a[4] = {
        0b11110000,
        0b10010000,
        0b10001101,
        0b10001000
    };
    u8encode w_a = 0;

    assert(u8str_get_encode_seq(&a[0], &w_a) == 4);
    assert(w_a == 0b11110000100100001000110110001000);

    byte_t b[3] = {
        0b11101101,
        0b10010101,
        0b10011100,
    };
    u8encode w_b = 0;

    assert(u8str_get_encode_seq(&b[0], &w_b) == 3);
    assert(w_b == 0b11101101100101011001110000000000);

    byte_t c[2] = {
        0b11010000,
        0b10011000,
    };
    u8encode w_c = 0;

    assert(u8str_get_encode_seq(&c[0], &w_c) == 2);
    assert(w_c == 0b11010000100110000000000000000000);

    byte_t d[1] = {
        0b00010010,
    };
    u8encode w_d = 0;

    assert(u8str_get_encode_seq(&d[0], &w_d) == 1);
    assert(w_d == 0b00010010000000000000000000000000);

    byte_t e[1] = {
        0b10010010,
    };
    u8encode w_e = 42069;

    assert(w_e == 42069);
    assert(!u8str_get_encode_seq(&e[0], &w_e));

    return 0;
}

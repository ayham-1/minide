/*
 * Simple UTF-8 implementation for minide.
 * Does not automatically verify UTF-8 rules.
 * Does not check if memory is enough when reading UTF-8 sequences.
 * Little-endian support only.
 * */
#ifndef U8_STRING_H
#define U8_STRING_H

#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>

#define WCHAR_BITS 32

typedef uint32_t wbyte_t;
typedef uint8_t byte_t;

typedef struct {
    size_t size;
    size_t clen;
    byte_t bytes[];
} string_t;

string_t* u8str_create(size_t size);
void u8str_cleanup(string_t *str);

size_t u8str_get_bytes_needed_for(wbyte_t c);
size_t u8str_get_wbyte_seq(byte_t* ptr, wbyte_t* u);
size_t u8str_clen(string_t* str);

bool u8str_cmp(string_t* s1, string_t* s2);

/*
 * 0xC0 = 11000000
 * 0x80 = 10000000
 * x & 0xC0 != 0x80 -> is utf sequence start
 */
#define U8_4_BYTE_MASK 0b11110000
#define U8_3_BYTE_MASK 0b11100000
#define U8_2_BYTE_MASK 0b11000000
#define U8_1_BYTE_MASK 0b10000000

#define MAKE_BYTE_WBYTE(x) (x << 24)
#define COMBINE_BYTES_TO_WBYTE(a, b, c, d) (d | (c << 8) | (b << 16) | (a << 24))

#endif

/*
 * Simple UTF-8 implementation for minide.
 * TODO(ayham): Does not automatically verify UTF-8 rules.
 * TODO(ayham): Does not check if memory is enough when reading UTF-8 sequences.
 *
 * follows RFC3629
 * */
#ifndef U8_STRING_H
#define U8_STRING_H

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

typedef uint8_t byte_t;
typedef uint32_t u8encode;
typedef byte_t * u8cptr_t; // pointer with knowledge of UTF-8

typedef struct {
	size_t size;
	size_t clen;
	byte_t * bytes;
} string_t;

void u8str_create(string_t * p, size_t size);
void u8str_cleanup(string_t * p);

size_t u8str_get_bytes_needed_for(u8encode c);
size_t u8str_get_encode_seq(byte_t * ptr, u8encode * out);
size_t u8str_clen(string_t * str);

bool u8str_is_utf8_valid(u8encode c);

bool u8str_inc(string_t * s, u8cptr_t * ptr);
bool u8str_dec(string_t * s, u8cptr_t * ptr);

/* parses U+0000..U+10FFFF
 * required to be null-terminated
 * */
u8encode u8str_from_code_point(byte_t c[9]);

#define U8_4_BYTE_MASK 0b11110000
#define U8_3_BYTE_MASK 0b11100000
#define U8_2_BYTE_MASK 0b11000000
#define U8_1_BYTE_MASK 0b10000000

#define U8_CONT_BYTE_MASK 0b10000000

#define U8_CODEPOINT_MASK 0b111111

#define U8_BOM 0xEFBBBF

#define MAKE_BYTE_WBYTE(x) (x << 24)
#define COMBINE_BYTES_TO_WBYTE(a, b, c, d)                                     \
	(d | (c << 8) | (b << 16) | (a << 24))

#define IS_CONT_BYTE(b)                                                        \
	((b & U8_CONT_BYTE_MASK) == U8_CONT_BYTE_MASK) &&                      \
	    (((~b) & 0b01000000) == 0b01000000)

#define IS_TAIL_BYTE(b) (b >= 0x80 && b <= 0xBF)

#endif

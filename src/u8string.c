#include "u8string.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

void u8str_create(string_t* p, size_t size) {
    p->size = size;
    p->clen = 0;
    p->bytes = malloc(size);
}

void u8str_cleanup(string_t* p) {
    free(p->bytes);
}


size_t u8str_get_bytes_needed_for(wbyte_t c) {
    byte_t first = ((byte_t*) &c)[3];
    if ((~first & U8_1_BYTE_MASK) == U8_1_BYTE_MASK) return 1;

    if ((first & U8_4_BYTE_MASK) == U8_4_BYTE_MASK) return 4;
    if ((first & U8_3_BYTE_MASK) == U8_3_BYTE_MASK) return 3;
    if ((first & U8_2_BYTE_MASK) == U8_2_BYTE_MASK) return 2;

    return 0; // invalid utf-8 sequence
}

size_t u8str_get_wbyte_seq(byte_t* ptr, wbyte_t *u) {
    byte_t* src_ptr = ptr; // copy pointer

    size_t bytes_num = u8str_get_bytes_needed_for(MAKE_BYTE_WBYTE(*ptr));
    if (bytes_num == 4) {
        *u = COMBINE_BYTES_TO_WBYTE(
            src_ptr[0],
            src_ptr[1],
            src_ptr[2],
            src_ptr[3]
        );
    } else if (bytes_num == 3) {
        *u = COMBINE_BYTES_TO_WBYTE(
            src_ptr[0],
            src_ptr[1],
            src_ptr[2],
            0x0
        );
    } else if (bytes_num == 2) {
        *u = COMBINE_BYTES_TO_WBYTE(
            src_ptr[0],
            src_ptr[1],
            0x0,
            0x0
        );
    } else if (bytes_num == 1) {
        *u = COMBINE_BYTES_TO_WBYTE(
            src_ptr[0],
            0x0,
            0x0,
            0x0
        );
    } else {
        return 0;
    }

    return bytes_num;
}

size_t u8str_clen(string_t* str) {
    if (str->clen != 0) return str->clen;
    size_t clen = 0;

    u8cptr_t ptr = str->bytes;
    while (u8str_inc(str, &ptr)) clen++;

    clen++; // u8str_inc does not increment past last character

    return clen;
}

bool u8str_is_seq_valid(wbyte_t c) {
    byte_t* arr = ((byte_t*) &c);
    size_t seq_len = u8str_get_bytes_needed_for(arr[3]) ;

    /* check first byte */
    if (seq_len <= 1) false;

    /* check sequence bits */
    if (!IS_CONT_BYTE(arr[2])) return false;
    if (seq_len >= 3 && !IS_CONT_BYTE(arr[1])) return false;
    if (seq_len == 4 && !IS_CONT_BYTE(arr[0])) return false;

    return true;
}

bool u8str_inc(string_t* s, u8cptr_t* ptr) {
    assert(s->bytes <= *ptr);
    assert(*ptr <= s->bytes + s->size - 1);

    // refuse to increment outside of allocated memory 
    if (*ptr + 1 == s->bytes + s->size) return false;

    size_t seq_len = u8str_get_bytes_needed_for(MAKE_BYTE_WBYTE(**ptr));

    // refuse to increment outside of allocated memory 
    if (*ptr + seq_len > s->bytes + s->size - 1) return false;

    if (seq_len == 0) (*ptr)--;
    else (*ptr) += seq_len;

    return true;
}

bool u8str_dec(string_t* s, u8cptr_t* ptr) {
    assert(s->bytes <= *ptr);
    assert(*ptr <= s->bytes + s->size - 1);

    // refuse to decrement outside of allocated memory 
    if (*ptr == s->bytes) return false;

    if (IS_CONT_BYTE(**ptr)) {
        while (*ptr >= s->bytes && IS_CONT_BYTE(**ptr)) *ptr -= 1;
        *ptr -= 1;
    } else (*ptr)--;

    return true;
}

#include "u8string.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

string_t* u8str_create(size_t size) {
    string_t* ret = malloc(sizeof(size_t)  // size
                           + sizeof(size_t) // clen
                           + size); // bytes
    ret->size = size;
    ret->clen = 0;

    return ret;
}

void u8str_cleanup(string_t *str) {
    free(str);
}

bool u8str_cmp(string_t* s1, string_t* s2) {
    if (s1->size != s2-> size) return false;

    return true;
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

    byte_t* ptr = str->bytes;
    size_t traversed_size = 0;

    size_t bytes_traversed = 0;
    while (bytes_traversed <= str->size) {
        if (u8str_get_bytes_needed_for(MAKE_BYTE_WBYTE(*ptr)) != 1) {
            wbyte_t u;
            size_t u_len = u8str_get_wbyte_seq(ptr, &u);
            ptr += u_len;
            bytes_traversed += u_len;
        }
        else {
            ptr++;
            bytes_traversed++;
        };
        clen++;
    }
    return clen;
}
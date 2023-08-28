#include "u8string.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

string_t* u8str_create(size_t size) {
    string_t* ret = malloc(sizeof(size_t)       // size
                           + sizeof(size_t)     // clen
                           + size);             // bytes
    ret->size = size;
    ret->clen = 0;

    return ret;
}

string_t* u8str_resize(string_t *old_str, size_t new_size) {
    assert(new_size > old_str-> size);

    string_t* new_str = malloc(sizeof(size_t)       // size
                               + sizeof(size_t)     // clen
                               + new_size);         // bytes

    memcpy(new_str, old_str, old_str->size);
    new_str->size = new_size;
    new_str->clen = old_str->clen;

    u8str_cleanup(old_str);
    
    return new_str;
}

void u8str_cleanup(string_t *str) {
    free(str);
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


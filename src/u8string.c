#include "minide/u8string.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void u8str_create(string_t * p, size_t size)
{
	p->size = size;
	p->clen = 0;
	p->bytes = malloc(size);
}

void u8str_cleanup(string_t * p) { free(p->bytes); }

size_t u8str_get_bytes_needed_for(u8encode c)
{
	byte_t first = ((byte_t *)&c)[3];
	if ((~first & U8_1_BYTE_MASK) == U8_1_BYTE_MASK)
		return 1;

	if ((first & U8_4_BYTE_MASK) == U8_4_BYTE_MASK)
		return 4;
	if ((first & U8_3_BYTE_MASK) == U8_3_BYTE_MASK)
		return 3;
	if ((first & U8_2_BYTE_MASK) == U8_2_BYTE_MASK)
		return 2;

	return 0; // invalid utf-8 start byte
}

size_t u8str_get_encode_seq(byte_t * ptr, u8encode * out)
{
	byte_t * src_ptr = ptr; // copy pointer

	size_t bytes_num = u8str_get_bytes_needed_for(MAKE_BYTE_WBYTE(*ptr));
	if (bytes_num == 4) {
		*out = COMBINE_BYTES_TO_WBYTE(src_ptr[0], src_ptr[1], src_ptr[2], src_ptr[3]);
	} else if (bytes_num == 3) {
		*out = COMBINE_BYTES_TO_WBYTE(src_ptr[0], src_ptr[1], src_ptr[2], 0x0);
	} else if (bytes_num == 2) {
		*out = COMBINE_BYTES_TO_WBYTE(src_ptr[0], src_ptr[1], 0x0, 0x0);
	} else if (bytes_num == 1) {
		*out = COMBINE_BYTES_TO_WBYTE(src_ptr[0], 0x0, 0x0, 0x0);
	} else {
		return 0;
	}

	return bytes_num;
}

size_t u8str_clen(string_t * str)
{
	if (str->clen != 0)
		return str->clen;
	size_t clen = 0;

	u8cptr_t ptr = str->bytes;
	while (u8str_inc(str, &ptr))
		clen++;

	clen++; // u8str_inc does not increment past last character

	return clen;
}

bool u8str_is_utf8_valid(u8encode c)
{
	/* null still considered as string terminator
	 * NOTE(ayham): (pls don't depend on it tho) */
	if (c == 0x0)
		return true;

	byte_t a[4] = {c, c >> 8, c >> 16, c >> 24};
	byte_t * first = NULL;
	for (int i = 0; i < 4; i++) {
		if ((a[i] & 0b11000000) == 0b11000000 || a[i] <= 0x7F) {
			first = &a[i];
			break;
		}
	}
	if (first == NULL)
		return false;

	byte_t * second = first - 1;
	byte_t * third = first - 2;
	byte_t * fourth = first - 3;

	size_t seq_len = u8str_get_bytes_needed_for(MAKE_BYTE_WBYTE(*first));
	if (seq_len == 0)
		return false;

	/* reject *the* forbidden octet values */
	for (int i = 0; i < 4; i++) {
		if (a[i] == 0xC0)
			return false;
		if (a[i] == 0xC1)
			return false;
		if (0xF5 <= a[i] /* && a[i] <= 0xFF */)
			return false;
	}

	/* check appropriate ranges */
	switch (seq_len) {
	case 1:
		// UTF8-1      = %x00-7F
		if (!(/**first >= 0x00 &&*/ *first <= 0x7F))
			return false;
		else
			break;
	case 2:
		// UTF8-2      = %xC2-DF UTF8-tail
		if (!(*first >= 0xC2 && *first <= 0xDF) && !IS_TAIL_BYTE(*second))
			return false;
		else
			break;
	case 3:
		/*
		 * UTF8-3      = %xE0 %xA0-BF UTF8-tail / %xE1-EC 2( UTF8-tail )
		 * / %xED %x80-9F UTF8-tail / %xEE-EF 2( UTF8-tail )
		 */
		if (!((*first == 0xE0 && *second >= 0xA0 && *second <= 0xBF && IS_TAIL_BYTE(*third)) ||
		      (*first >= 0xE1 && *first <= 0xEC && IS_TAIL_BYTE(*second) && IS_TAIL_BYTE(*third)) ||
		      (*first == 0xED && *second >= 0x80 && *second <= 0x9F && IS_TAIL_BYTE(*third)) ||
		      (*first >= 0xEE && *first <= 0xEF && IS_TAIL_BYTE(*second) && IS_TAIL_BYTE(*third))))
			return false;
		else
			break;
	case 4:
		/*
		 * UTF8-4      = %xF0 %x90-BF 2( UTF8-tail ) / %xF1-F3 3(
		 * UTF8-tail ) / %xF4 %x80-8F 2( UTF8-tail )
		 */
		if (!((*first == 0xF0 && *second >= 0x90 && *second <= 0xBF && IS_TAIL_BYTE(*third) &&
		       IS_TAIL_BYTE(*fourth)) ||
		      (*first >= 0xF1 && *first <= 0xF3 && IS_TAIL_BYTE(*second) && IS_TAIL_BYTE(*third) &&
		       IS_TAIL_BYTE(*fourth)) ||
		      (*first == 0xF4 && *second >= 0x80 && *second <= 0x8F && IS_TAIL_BYTE(*third) &&
		       IS_TAIL_BYTE(*fourth))))
			return false;
		else
			break;
	case 0:
		return false;
	default:
		break;
	};
	return true;
}

bool u8str_inc(string_t * s, u8cptr_t * ptr)
{
	assert(s->bytes <= *ptr);
	assert(*ptr <= s->bytes + s->size - 1);

	// refuse to increment outside of allocated memory
	if (*ptr + 1 == s->bytes + s->size)
		return false;

	size_t seq_len = u8str_get_bytes_needed_for(MAKE_BYTE_WBYTE(**ptr));

	// refuse to increment outside of allocated memory
	if (*ptr + seq_len > s->bytes + s->size - 1)
		return false;

	if (seq_len == 0)
		(*ptr)--;
	else
		(*ptr) += seq_len;

	return true;
}

bool u8str_dec(string_t * s, u8cptr_t * ptr)
{
	assert(s->bytes <= *ptr);
	assert(*ptr <= s->bytes + s->size - 1);

	// refuse to decrement outside of allocated memory
	if (*ptr == s->bytes)
		return false;

	if (IS_CONT_BYTE(**ptr)) {
		while (*ptr >= s->bytes && IS_CONT_BYTE(**ptr))
			*ptr -= 1;
		*ptr -= 1;
	} else
		(*ptr)--;

	return true;
}

u8encode u8str_from_code_point(byte_t c[9])
{
	if (strlen((char *)c) < 6)
		return 0x0;
	if (!(c[0] == 'U' && c[1] == '+'))
		return 0x0;

	const char * cptr = (char *)&c[2];
	u8encode rawByte = strtoull(cptr, NULL, 16);

	byte_t a[4] = {0};
	/*
	 * U+4E3E
	 * 1110xxxx 10xxxxxx 10xxxxxx
	 * 0x4E3E -> 0b100111000111110
	 *
	 * 1110xxxx 10xxxxxx 10xxxxxx
	 *      100   111000   111110  (OR)
	 * ---------------------------
	 * 1110x100 10111000 10111110
	 */
	size_t seq_len = 0;
	if (rawByte <= 0x7F)
		seq_len = 1;
	else if (rawByte <= 0x7FF)
		seq_len = 2;
	else if (rawByte <= 0xFFFF)
		seq_len = 3;
	else if (rawByte <= 0x1FFFFF)
		seq_len = 4;

	for (size_t b_index = 0; b_index < seq_len; b_index++) {
		a[b_index] = (rawByte >> (6 * (seq_len - b_index - 1))) & (U8_CODEPOINT_MASK);
		if (b_index == 0) {
			/* control byte */
			a[0] &= 0b00000111; // clear utf-8 control bits
			if (seq_len == 4)
				a[0] |= U8_4_BYTE_MASK;
			else if (seq_len == 3)
				a[0] |= U8_3_BYTE_MASK;
			else if (seq_len == 2)
				a[0] |= U8_2_BYTE_MASK;
			else if (seq_len == 1)
				a[0] |= U8_1_BYTE_MASK;
		} else {
			a[b_index] &= 0b00111111; // clear utf-8 sequence control bits
			a[b_index] |= U8_CONT_BYTE_MASK;
		}
	}

	return COMBINE_BYTES_TO_WBYTE(a[0], a[1], a[2], a[3]);
}

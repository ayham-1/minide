#include <assert.h>

#include "../src/u8string.h"

int main(int argc, char * argv[])
{
	assert(u8str_get_bytes_needed_for(MAKE_BYTE_WBYTE(0b00100100) == 1));
	assert(u8str_get_bytes_needed_for(MAKE_BYTE_WBYTE(0b11000010) == 2));
	assert(u8str_get_bytes_needed_for(MAKE_BYTE_WBYTE(0b11100000) == 3));
	assert(u8str_get_bytes_needed_for(MAKE_BYTE_WBYTE(0b11110000) == 4));
	assert(u8str_get_bytes_needed_for(MAKE_BYTE_WBYTE(0b10000000) == 0));
	assert(u8str_get_bytes_needed_for(MAKE_BYTE_WBYTE(0b01000000) == 0));
	return 0;
}

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/u8string.h"

#define HELLO "HELLO 👋 WORLD 🌏"
#define HELLO_BYTES_NUM sizeof(HELLO) - 1
#define HELLO_LEN 15

int main(int argc, char * argv[])
{
	string_t ustr;
	u8str_create(&ustr, HELLO_BYTES_NUM);
	memcpy(ustr.bytes, HELLO, HELLO_BYTES_NUM);

	size_t incs = 0;

	u8cptr_t ptr = ustr.bytes;
	while (u8str_inc(&ustr, &ptr))
		incs++;

	// '- 1' -> must not increment past last character
	assert(incs == HELLO_LEN - 1);
	assert(*ptr == 0b11110000);

	u8str_cleanup(&ustr);
	return 0;
}

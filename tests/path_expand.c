#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/logger.h"
#include "../src/path.h"

#define PATH "~/"
#define PATH_BYTES_NUM sizeof(PATH)

int main(int argc, char * argv[])
{
	logger_init(DEBUG, "", false);

	path_t p;
	path_create(&p, PATH_BYTES_NUM);
	memcpy(p.fullPath.bytes, PATH, PATH_BYTES_NUM);

	assert(path_expand(&p));

	path_cleanup(&p);
	logger_cleanup();
	return 0;
}

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "minide/logger.h"
#include "minide/path.h"

#define PATH "~/"
#define PATH_BYTES_NUM sizeof(PATH)

int main(int argc, char * argv[])
{
	logger_init(DEBUG, "", false);

	// WARNING: ALL STRING RETURNS ARE NEW MALLOCS,
	// PLEASE DO NOT FORGET TO FREE RETURNED CHAR * PTRS
	// here we don't care about memory as the entirety of
	// the program is a couple of lines long

	assert(!path_is_real("alfjkads"));
	log_debug("path_get_absolute(\"test\"): % s ", path_get_absolute("test"));
	log_debug("path_get_absolute(\"../test_path\"): %s", path_get_absolute("../test_path"));
	log_var(path_is_real(path_get_absolute("../test_path")));
	log_var(path_is_real("../test_path"));

	log_var(path_get_type("../test_path") == file_type_regular);
	log_var(path_can_read("../test_path") == true);

	log_debug("path_get_name(path_get_absolute(\"../test_path\")): %s",
		  path_get_name(path_get_absolute("../test_path")));

	log_debug("path_get_extension(\"../test_path.exelol\"): %s", path_get_extension("../test_path.exelol"));

	logger_cleanup();
	return 0;
}

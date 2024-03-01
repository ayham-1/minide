/*
 * Basic cross-platform all path-related things.
 * Assumes logger.h is initialized.
 * */
#ifndef PATH_H
#define PATH_H

#include "minide/logger.h"

typedef enum {
	file_type_none,
	file_type_not_found,
	file_type_regular,
	file_type_directory,
	file_type_symlink,
	file_type_block,
	file_type_character,
	file_type_fifo,
	file_type_socket,
	file_type_unknown,
} file_type_t;

bool path_create_file(char * p);
bool path_create_dir(char * p);

bool path_is_real(char * p);
bool path_can_read(char * p);
char * path_get_name(char * p);
char * path_get_extension(char * p);
char * path_get_absolute(char * p);
file_type_t path_get_type(char * p);

#endif

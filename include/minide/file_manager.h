#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "minide/file_interface.h"
#include "minide/logger.h"

#define FILE_MANAGER_INITIAL_CAPACITY 5

typedef bool (*file_closing_function)(void * handle);

typedef struct {
	const char * name;
	const char * mode;
	FILE * fp;
	file_closing_function clean_fn;
} file_handle;

typedef struct {
	size_t capacity;
	file_handle * handles;
	bool * registered;
} file_manager;

void file_manager_init();
void file_manager_cleanup();
void file_manager_clean_file_handle(file_handle * handle);

file_handle * file_manager_get_handle_by_index(size_t index);
int32_t file_manager_register(const char * name, const char * mode, file_closing_function clean_fn);
void file_manager_unregister(size_t index);

#endif

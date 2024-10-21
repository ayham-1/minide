#include "minide/file_manager.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

file_manager __f_man = {0};

void file_manager_init()
{
	__f_man.capacity = FILE_MANAGER_INITIAL_CAPACITY;
	__f_man.handles = malloc(sizeof(file_handle *) * __f_man.capacity);
	__f_man.registered = malloc(sizeof(bool) * __f_man.capacity);
	memset(__f_man.registered, false, __f_man.capacity);
}

void file_manager_cleanup()
{
	for (size_t i = 0; i < __f_man.capacity; i++) {
		if (!__f_man.registered[i])
			continue;

		file_handle * handle = &__f_man.handles[i];
		file_manager_clean_file_handle(handle);
	}

	free(__f_man.handles);
	free(__f_man.registered);
}

void file_manager_clean_file_handle(file_handle * handle)
{
	if (!handle->clean_fn) {
		log_warn("file name: %s", handle->name);
		log_warn("file mode: %s", handle->mode);
		log_warn("does not appear to have a set clean function");
		log_warn("it is very likely something very wrong happened and we are leaking memory right now");
		return;
	}

	if (!handle->clean_fn(handle)) {
		log_warn("file name: %s", handle->name);
		log_warn("file mode: %s", handle->mode);
		log_warn("clean_fn() did not return true");
	}
	fclose(handle->fp);
}

file_handle * file_manager_get_handle_by_index(size_t index)
{
	assert(index >= 0);
	assert(index < __f_man.capacity);
	if (__f_man.registered[index])
		return &__f_man.handles[index];
	else
		return NULL;
}

int32_t file_manager_register(const char * name, const char * mode, file_closing_function clean_fn)
{
	assert(name);
	assert(mode);
	assert(clean_fn);

	file_handle * handle = NULL;
	int32_t index = -1;

handle_search:
	for (size_t i = 0; i < __f_man.capacity; i++) {
		if (!__f_man.registered[i]) {
			handle = &__f_man.handles[i];
			index = i;
			break;
		}
	}
	if (handle == NULL && index == -1) {
		int32_t old_cap = __f_man.capacity;
		__f_man.capacity *= 2;
		__f_man.handles = realloc(__f_man.handles, sizeof(file_handle) * __f_man.capacity);
		__f_man.registered = realloc(__f_man.registered, sizeof(bool) * __f_man.capacity);
		memset(__f_man.registered + old_cap, false, __f_man.capacity - old_cap);
		goto handle_search;
	}

	handle->name = name;
	handle->mode = mode;
	handle->fp = fopen(name, mode);
	handle->clean_fn = clean_fn;

	return index;
}

void file_manager_unregister(size_t index)
{
	assert(index >= 0);
	assert(index < __f_man.capacity);
	if (!__f_man.registered[index]) {
		log_warn("requested to unregister a file_handle that is not registered");
		log_warn("is something very bad happening?");
	}
	__f_man.registered[index] = false;
}

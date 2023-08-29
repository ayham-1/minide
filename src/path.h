#ifndef PATH_H
#define PATH_H

#include "u8string.h"

typedef struct {
    string_t* fullPath;
} path_t;

path_t* path_create(size_t size);
void path_cleanup(path_t* p);

void path_sanitize(path_t* p);
void path_expand(path_t* p);

bool path_traverse(path_t* p);

bool path_is_real(path_t* p);
bool path_can_access(path_t* p);
string_t* path_get_name(path_t* p);
string_t* path_get_extension(path_t* p);
string_t* path_get_type(path_t* p);

#endif

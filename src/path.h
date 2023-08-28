#ifndef PATH_H
#define PATH_H

#include "u8string.h"

typedef struct {
    size_t pathSize;
    string_t* fullPath;
} path_t;

path_t* path_create(size_t size);
void path_cleanup(path_t* p);

void path_sanitize(path_t* p);

#endif

#include "path.h"

#include <stdlib.h>
#include <string.h>

path_t* path_create(size_t size) {
    path_t* ptr = malloc(sizeof(path_t));

    ptr->fullPath = u8str_create(size);

    return ptr;
}

void path_cleanup(path_t* p) {
    u8str_cleanup(p->fullPath);

    free(p);
}

void path_sanitize(path_t* p) {
}

void path_expand(path_t* p);

bool path_traverse(path_t* p);

bool path_is_real(path_t* p);
bool path_can_access(path_t* p);
string_t* path_get_name(path_t* p);
string_t* path_get_extension(path_t* p);
string_t* path_get_type(path_t* p);

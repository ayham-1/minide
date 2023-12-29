/*
 * Basic cross-platform all path-related things.
 * Assumes logger.h is initialized.
 * */
#ifndef PATH_H
#define PATH_H

#include "logger.h"
#include "u8string.h"

typedef struct {
    string_t fullPath;
} path_t;

void path_create(path_t *p, size_t size);
void path_cleanup(path_t *p);

bool path_expand(path_t *p);

bool path_is_real(path_t *p);
bool path_can_access(path_t *p);
string_t *path_get_name(path_t *p);
string_t *path_get_extension(path_t *p);
string_t *path_get_type(path_t *p);

#endif

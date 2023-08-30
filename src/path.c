#include "path.h"

// TODO(ayham): when writing cross-platform impl. fix includes
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

void path_create(path_t* p, size_t size) {
    p->fullPath = u8str_create(size);
}

void path_cleanup(path_t* p) {
    u8str_cleanup(p->fullPath);
}

bool path_expand(path_t* p) {
#ifdef __linux__
    byte_t* new_path = (byte_t*)realpath((char*)p->fullPath->bytes, NULL);
    if (new_path == NULL) {
        log_error("path: %s, couldn't be expanded by system, error: %s", p->fullPath->bytes, strerror(errno));
    } else {
        u8str_cleanup(p->fullPath);
        byte_t* ptr = new_path;
        while (*ptr != '\0') ptr++;
        p->fullPath = u8str_create(ptr - new_path);
        memcpy(new_path, p->fullPath->bytes, ptr - new_path); // maybe this can be done easier 
    }
    return true;
#else
    log_warn("HAHA, you don't get files!!!!");
    return false;
#endif
}

bool path_traverse(path_t* p);

bool path_is_real(path_t* p);
bool path_can_access(path_t* p);
string_t* path_get_name(path_t* p);
string_t* path_get_extension(path_t* p);
string_t* path_get_type(path_t* p);

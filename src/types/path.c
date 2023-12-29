#include "path.h"

// TODO(ayham): when writing cross-platform impl. fix includes
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

void path_create(path_t *p, size_t size) { u8str_create(&p->fullPath, size); }

void path_cleanup(path_t *p) { u8str_cleanup(&p->fullPath); }

bool path_expand(path_t *p) {
#ifdef __linux__
  byte_t *new_path = (byte_t *)realpath((char *)p->fullPath.bytes, NULL);
  if (new_path == NULL) {
    log_error("path: %s, couldn't be expanded by system. error: %s",
              p->fullPath.bytes, strerror(errno));
    return false;
  } else {
    u8str_cleanup(&p->fullPath);
    byte_t *ptr = new_path;
    while (*ptr != '\0')
      ptr++;
    u8str_create(&p->fullPath, ptr - new_path);
    memcpy(new_path, p->fullPath.bytes,
           ptr - new_path); // maybe this can be done easier
    free(new_path); // man 3 realpath says PATH_MAX is allocated on return, cpy
                    // and free
    return true;
  }
#else
  log_warn("HAHA, you don't get files!!!!");
  return false;
#endif
}

bool path_is_real(path_t *p);
bool path_can_access(path_t *p);
string_t *path_get_name(path_t *p);
string_t *path_get_extension(path_t *p);
string_t *path_get_type(path_t *p);

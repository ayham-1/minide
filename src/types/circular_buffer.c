#include "circular_buffer.h"

#define _GNU_SOURCE
#include <sys/mman.h>

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "../logger.h"

bool cbuffer_create(cbuffer_t* cbuf, 
                    size_t capacity, size_t entrySz,
                    const char* bname) {
    cbuf->bname = malloc(strlen(bname));
    strcpy(cbuf->bname, bname);

    cbuf->capacity = capacity;
    cbuf->entrySz = entrySz;

    size_t page_size = getpagesize();
    size_t byte_size = (capacity * entrySz) / page_size;
    if (byte_size * page_size != capacity * entrySz)
        byte_size++;
    cbuf->sz = byte_size * page_size;

    assert(cbuf->sz > entrySz); // something really wrong happened
 
    /* update capacity after page alignment */
    cbuf->capacity = cbuf->sz / entrySz;
    
    cbuf->_fd = memfd_create(cbuf->bname, 0);
    ftruncate(cbuf->_fd, byte_size);

    cbuf->buf = mmap(NULL, 2 * cbuf->sz, PROT_NONE, 
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (cbuf->buf == MAP_FAILED) {
        log_error("buffer %s, failed allocation mapping.\n\terror: %s",
                  cbuf->bname,
                  strerror(errno));
        return false;
    }

    void* result = NULL;
    result = mmap(cbuf->buf, cbuf->sz, 
               PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_FIXED, cbuf->_fd, 0);
    if (result == MAP_FAILED) {
        log_error("buffer %s, failed initial mapping.\n\terror: %s",
                  cbuf->bname,
                  strerror(errno));
        return false;
    }

    result = mmap(cbuf->buf + cbuf->sz, 
               byte_size, PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_FIXED, cbuf->_fd, 0);
    if (result == MAP_FAILED) {
        log_error("buffer %s, failed circular mapping.\n\terror: %s",
                  cbuf->bname,
                  strerror(errno));
        return false;
    }

    cbuf->head = 0;
    cbuf->tail = 0;

    log_info("buffer %s, allocated with %i bytes total, with entry capacity of %i" ,
             cbuf->bname, cbuf->sz, cbuf->capacity);

    return true;
}
void cbuffer_cleanup(cbuffer_t* cbuf) {
    if (munmap(cbuf->buf, cbuf->sz) == -1) {
        log_error("buffer %s, failed unmapping.\n\terror: %s",
                  cbuf->bname,
                  strerror(errno));
    }

    free(cbuf->bname);
}

bool cbuffer_put(cbuffer_t* cbuf, uint8_t* data) {
    memcpy(&cbuf->buf[cbuf->tail], data, cbuf->entrySz);
    cbuf->tail += cbuf->entrySz;

    if (cbuf->tail > cbuf->sz)
        cbuf->tail -= cbuf->sz;

    return true;
}
bool cbuffer_pop(cbuffer_t* cbuf, uint8_t* data) {
    memcpy(data, &cbuf->buf[cbuf->head], cbuf->entrySz);
    cbuf->head += cbuf->entrySz;
    
    if (cbuf->head > cbuf->sz)
        cbuf->head -= cbuf->sz;

    return true;
}

bool cbuffer_get_by_index(cbuffer_t* cbuf, uint8_t* data, 
                          size_t index) {
    if (index > (cbuf->sz))
        return false;

    return true;
}

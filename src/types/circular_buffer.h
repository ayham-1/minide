#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t* buf;
    size_t capacity;
    size_t entrySz;
    size_t sz;
    size_t head;
    size_t tail;
    int _fd;

    char* bname;
} cbuffer_t;

bool cbuffer_create(cbuffer_t* cbuf, 
                    size_t capacity, size_t entrySz,
                    const char* bname);
void cbuffer_cleanup(cbuffer_t* cbuf);

bool cbuffer_put(cbuffer_t* cbuf, uint8_t* data);
bool cbuffer_pop(cbuffer_t* cbuf, uint8_t* data);

bool cbuffer_get_by_index(cbuffer_t* cbuf, uint8_t* data, 
                          size_t index);

#endif

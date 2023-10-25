#include "vector.h"

#include <malloc.h>
#include <assert.h>

void vec_create(vec_t* v, size_t capacity, 
                vec_resize_handler resize_handler, uint8_t* resize_handler_data) {
    v->items = malloc(sizeof(uint8_t*) * capacity);
    v->count = 0;
    v->capacity = capacity;
    v->resize_fun = resize_handler;
}

void vec_cleanup(vec_t* v) {
    free(v->items);
}

void vec_resize(vec_t* v, size_t new_capacity) {
    v->items = realloc(v->items, sizeof(uint8_t*) * new_capacity);
    v->capacity = new_capacity;
    v->resize_fun(v, v->resize_data);
}

void vec_append(vec_t* v, uint8_t* item) {
    assert(item != NULL); // no

    if (v->count + 1 >= v->capacity) {
        vec_resize(v, 2 * v->capacity);
    }
    v->items[v->count++] = item;
}

uint8_t* vec_replace(vec_t* v, uint8_t* item, size_t index) {
    assert(item != NULL);
    // user must manager those fragmented replaces
    // if resize happens in the future
    assert(index <= v->capacity);

    uint8_t* old_pointer = v->items[index];

    v->items[index] = item;

    return old_pointer;
}

uint8_t* vec_get(vec_t* v, size_t index) {
    assert(index <= v->capacity);
    return v->items[index];
}

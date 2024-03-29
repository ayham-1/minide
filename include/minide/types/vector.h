#ifndef VECTOR_H
#define VECTOR_H

#include <stdint.h>
#include <unistd.h>

typedef struct {
	uint8_t ** items;
	uint8_t * data;
	size_t count;
	size_t capacity;
	size_t sizeof_item;
} vec_t;

void vec_create(vec_t * v, size_t capacity, size_t szof_item);
void vec_cleanup(vec_t * v);
void vec_resize(vec_t * v, size_t new_capacity);

void vec_append(vec_t * v, uint8_t * item);
uint8_t * vec_replace(vec_t * v, uint8_t * item, size_t index);

uint8_t * vec_get(vec_t * v, size_t index);

#endif

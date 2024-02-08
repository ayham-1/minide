/*
 * Provides a simple implementation of vector type.
 *
 * This data structure uses pointers to data provided
 * by the user.
 *
 * The user is *not* expected to malloc every single item,
 * but rather point the individual items into a large
 * memory allocation managed by the user.
 *
 * The user is however expected to remap the items pointers
 * when a resize happens. The provided function is automatically
 * called whenever a resize is needed. It is expected for resizes
 * to be a outleast a couple of multiples of big-o of N, O(n),
 * therefore user should be mindful about the initial capacity.
 *
 * The user is also expected to take care of the non-existent
 * differentiation between count and capacity when using the
 * provided functions. Fragmentation of the `items` array
 * is plausible with `vec_replace()` and `vec_insert()`
 * usage.
 *
 * */
#ifndef VECTOR_H
#define VECTOR_H

#include <stdint.h>
#include <unistd.h>

typedef struct vec_t vec_t;
typedef void (*vec_resize_handler)(vec_t * v, uint8_t * data);

struct vec_t {
	uint8_t ** items;
	size_t count;
	size_t capacity;

	vec_resize_handler resize_fun;
	uint8_t * resize_data;
};

void vec_create(vec_t * v, size_t capacity, vec_resize_handler resize_handler, uint8_t * resize_handler_data);
void vec_cleanup(vec_t * v);
void vec_resize(vec_t * v, size_t new_capacity);

void vec_append(vec_t * v, uint8_t * item);
uint8_t * vec_replace(vec_t * v, uint8_t * item, size_t index);

uint8_t * vec_get(vec_t * v, size_t index);

#endif

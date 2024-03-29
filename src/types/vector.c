#include "minide/types/vector.h"

#include <assert.h>
#include <malloc.h>
#include <string.h>

void vec_create(vec_t * v, size_t capacity, size_t szof_item)
{
	v->items = malloc(sizeof(uint8_t *) * capacity);
	memset(v->items, 0, sizeof(uint8_t *) * capacity);
	v->data = malloc(szof_item * capacity);
	memset(v->data, 0, szof_item * capacity);

	v->count = 0;
	v->capacity = capacity;
	v->sizeof_item = szof_item;
}

void vec_cleanup(vec_t * v)
{
	free(v->items);
	free(v->data);
	v->count = 0;
	v->capacity = 0;
	v->sizeof_item = 0;
}

void vec_resize(vec_t * v, size_t new_capacity)
{
	v->items = realloc(v->items, sizeof(uint8_t *) * new_capacity);
	v->data = realloc(v->data, v->sizeof_item * new_capacity);
	for (size_t i = 0; i < v->capacity; i++)
		v->items[i] = &v->data[i * v->sizeof_item];
	v->capacity = new_capacity;
}

void vec_append(vec_t * v, uint8_t * item)
{
	assert(item != NULL); // no

	if (v->count + 1 >= v->capacity) {
		vec_resize(v, 2 * v->capacity);
	}
	v->items[v->count++] = item;
}

uint8_t * vec_replace(vec_t * v, uint8_t * item, size_t index)
{
	assert(item != NULL);
	// user must manager those fragmented replaces
	// if resize happens in the future
	assert(index <= v->capacity);

	uint8_t * old_pointer = v->items[index];

	v->items[index] = item;

	return old_pointer;
}

uint8_t * vec_get(vec_t * v, size_t index)
{
	assert(index <= v->capacity);
	return v->items[index];
}

#include "minide/types/stack.h"

#include <assert.h>
#include <malloc.h>
#include <string.h>

#define STACK_DEFAULT_SIZE 5
#define STACK_REALLOC_FACTOR 2

void stack_create(stack_t * s, size_t capacity)
{
	assert(s);
	s->capacity = STACK_DEFAULT_SIZE;
	s->count = 0;
	s->data = malloc(s->capacity * sizeof(int8_t));
}

void stack_clean(stack_t * s)
{
	assert(s);
	free(s->data);
	s->data = NULL;
	s->count = 0;
	s->capacity = 0;
}

void stack_push(stack_t * s, uint8_t item)
{
	assert(s);
	assert(s->capacity && s->count && s->data);

	if (s->count + 1 >= s->capacity) {
		s->capacity *= STACK_REALLOC_FACTOR;
		s->data = realloc(s->data, s->capacity * sizeof(int8_t));
	}

	s->data[s->count] = item;

	s->count++;
}

uint8_t stack_pop(stack_t * s)
{
	assert(s);
	assert(s->capacity && s->count && s->data);

	s->count--;
	return s->data[s->count];
}

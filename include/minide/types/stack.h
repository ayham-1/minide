#ifndef STACK_H
#define STACK_H

#include <stdint.h>
#include <unistd.h>

typedef struct {
	uint8_t * data;
	size_t count;
	size_t capacity;
} stack_t;

void stack_create(stack_t * s, size_t capacity);
void stack_clean(stack_t * s);

void stack_push(stack_t * s, uint8_t item);
uint8_t stack_pop(stack_t * s);

#endif

#ifndef GENERAL_ALLOCATOR
#define GENERAL_ALLOCATOR

#include <stdbool.h>
#include <unistd.h>

typedef struct {
	size_t byte_cnt;
	size_t item_byte_cnt;
	void * chunk;

	bool * is_allocated;
} general_allocator_t;

void general_allocator_init(general_allocator_t *);
void general_allocator_clean(general_allocator_t *);

void * general_allocator_alloc(general_allocator_t *, size_t cnt);
void general_allocator_free(general_allocator_t *, void *);

#endif

#include "minide/general_alloc.h"

#include <assert.h>

void general_allocator_init(general_allocator_t * alloc)
{
	assert(alloc);
	assert(alloc->byte_cnt);
	alloc->chunk = malloc();
}
void general_allocator_clean(general_allocator_t *);

void * general_allocator_alloc(general_allocator_t *, size_t cnt);
void general_allocator_free(general_allocator_t *, void *);

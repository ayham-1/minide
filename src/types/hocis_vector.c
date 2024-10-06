#include "minide/types/hocis_vector.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "minide/logger.h"

#define HOCIS_VEC_INITIAL_ITEM_COUNT 8
#define HOCIS_VEC_RESIZE_FACTOR 2

#define HOCIS_ASSERT_ITEM_BELONGS(v, item)                                                                             \
	assert(v->mem <= item);                                                                                        \
	assert(v->mem + v->mem_sz > item);

void hocis_vec_create(hocis_vec_t * v, size_t initial_item_count, size_t size_of_item)
{
	assert(v);
	assert(size_of_item > 0);

	if (initial_item_count <= HOCIS_VEC_INITIAL_ITEM_COUNT)
		initial_item_count = HOCIS_VEC_INITIAL_ITEM_COUNT;

	v->item_count = initial_item_count;
	v->item_sz = size_of_item;

	v->mem_sz = v->item_count * (sizeof(hocis_vec_item_t) + v->item_sz);
	v->mem = malloc(v->mem_sz);

	if (!v->mem) {
		log_error("Failed to allocate memory for hocis_vec_t");
	}

	memset(v->mem, 0, v->mem_sz);

	v->item_used_count = 0;
	v->item_free_count = v->item_count;

	v->ind_first_free_item = 0;
}

void hocis_vec_free(hocis_vec_t * v)
{
	assert(v);
	free(v->mem);
	memset(v, 0, sizeof(hocis_vec_t));
}

void * hocis_vec_append(hocis_vec_t * v)
{
	assert(v);

	if (v->item_free_count == 0)
		__hocis_vec_resize(v, v->item_count * HOCIS_VEC_RESIZE_FACTOR);

	hocis_vec_item_t * first_free_item = hocis_vec_get_from_ind(v, v->ind_first_free_item);

	// check if item is free
	if (first_free_item->is_used) {
		log_error("something went very wrong when resizing the vector");
		exit(1);
	}

	// update next free item to not have a prev index
	if (first_free_item->index_next_free) { // if 0: end of the free items list
		hocis_vec_item_t * second_free_item = hocis_vec_get_from_ind(v, first_free_item->index_next_free);
		assert(first_free_item->is_used == false);
		second_free_item->index_prev_free = 0;

		v->ind_first_free_item = first_free_item->index_next_free;
	} else { // we just filled last free item
		assert(v->item_free_count == 1);
		v->ind_first_free_item = 0;
	}

	// item is now in use, reflect meta variables
	first_free_item->is_used = true;
	first_free_item->index_prev_free = 0;
	first_free_item->index_next_free = 0;

	v->item_free_count -= 1;
	v->item_used_count += 1;

	return hocis_vec_data_from_item(first_free_item);
}

void hocis_vec_remove(hocis_vec_t * v, hocis_vec_item_t * item)
{
	assert(v);
	assert(item);

	// we check if the caller is trolling us
	assert(item->is_used);
	HOCIS_ASSERT_ITEM_BELONGS(v, item);

	size_t index_to_be_freed = hocis_vec_get_ind(v, item);
	if (index_to_be_freed < v->ind_first_free_item) { // simple case of just appending to the front of the
							  // double-linked list of free blocks
		hocis_vec_item_t * old_first_free = hocis_vec_get_from_ind(v, v->ind_first_free_item);
		old_first_free->index_prev_free = index_to_be_freed;
		v->ind_first_free_item = index_to_be_freed;
	} else { // iterate till we find closest free item
		 // iterate both ways a the same
	}

	v->item_free_count += 1;
	v->item_used_count -= 1;
}

hocis_vec_item_t * hocis_vec_get_from_ind(hocis_vec_t * v, size_t index)
{
	assert(v);

	size_t offset_bytes = index * (sizeof(hocis_vec_item_t) + v->item_sz);

	return v->mem + offset_bytes;
}

hocis_vec_item_t * hocis_vec_get_from_data(hocis_vec_t * v, void * data)
{
	/*
	 * since we malloc the v->mem with size of:
	 * 	v->item_count * (sizeof(hocis_vec_item_t) + v->item_sz),
	 * the holding hocis_vec_item_t pointer is just the subtraction of the size of the hocis_vec_t
	 * from the data
	 */

	return data - sizeof(hocis_vec_item_t);
}

void * hocis_vec_data_from_item(hocis_vec_item_t * i)
{
	/*
	 * since we malloc the v->mem with size of:
	 * 	v->item_count * (sizeof(hocis_vec_item_t) + v->item_sz),
	 * the individual data pointer is just the addition of the size of the hocis_vec_item_t
	 */
	assert(i->is_used);
	return i + sizeof(hocis_vec_item_t);
}

size_t hocis_vec_get_ind(hocis_vec_t * v, hocis_vec_item_t * item)
{
	assert(item->is_used);
	HOCIS_ASSERT_ITEM_BELONGS(v, item);
	return (item - v->mem) / sizeof(hocis_vec_item_t);
}

void __hocis_vec_resize(hocis_vec_t * v, size_t new_item_count)
{
	assert(v);
	assert(v->item_count < new_item_count);

	v->item_count = new_item_count;
	v->item_free_count = v->item_count - v->item_used_count;

	size_t new_sz = v->item_count * (sizeof(hocis_vec_item_t) + v->item_sz);

	v->mem = realloc(v->mem, v->mem_sz);
	memset(v->mem + v->mem_sz, 0, new_sz - v->mem_sz);

	v->mem_sz = new_sz;

	// update first index of free item if needed
	if (hocis_vec_get_from_ind(v, v->ind_first_free_item)->is_used) {
		// the vector was full, update ind_first_free_item
		size_t new_ind = v->item_used_count + 1;
		if (hocis_vec_get_from_ind(v, new_ind)->is_used) {
			log_error("could not find the first non-free item after resizing");
			exit(1);
		}

		v->ind_first_free_item = new_ind;
	}

	/* chain the new non used items */

	// find the last non used item
	size_t last_non_used_ind = v->ind_first_free_item;
	while (hocis_vec_get_from_ind(v, last_non_used_ind)->index_next_free) {
		last_non_used_ind = hocis_vec_get_from_ind(v, last_non_used_ind)->index_next_free;
	}

	// chain the new non used item
	hocis_vec_get_from_ind(v, last_non_used_ind)->index_next_free = last_non_used_ind + 1;
	hocis_vec_get_from_ind(v, last_non_used_ind + 1)->index_prev_free = last_non_used_ind;

	for (size_t ind = last_non_used_ind + 2; ind < v->item_count; ind++) {
		hocis_vec_item_t * crnt = hocis_vec_get_from_ind(v, ind);
		hocis_vec_item_t * prev = hocis_vec_get_from_ind(v, ind - 1);

		crnt->index_prev_free = ind - 1;
		prev->index_next_free = ind;
	}

	if (!v->mem) {
		log_error("Failed to resize hocis_vec_t");
	}
}

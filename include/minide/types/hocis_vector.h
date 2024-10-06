// hocis_vector: heap-allocated optimized constant item size vector

#ifndef HOCIS_VECTOR_H
#define HOCIS_VECTOR_H

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

typedef struct hocis_vec_item_t hocis_vec_item_t;

struct hocis_vec_item_t {
	/* meta variables */
	bool is_used;
	size_t index_next_free;
	size_t index_prev_free;

	/*
	 * There is no guarantee, that on resize, a pointer stored here
	 * would stay the same after realloc(). Therefore we just calculate
	 * the pointer to the data belonging to the item ourselves whenever
	 * it is needed.
	 * */
};

typedef struct {
	hocis_vec_item_t * mem;
	size_t mem_sz;

	size_t item_count;
	size_t item_sz;

	size_t item_used_count;
	size_t item_free_count;

	size_t ind_first_free_item;
} hocis_vec_t;

void hocis_vec_create(hocis_vec_t * v, size_t initial_item_count, size_t size_of_item);
void hocis_vec_free(hocis_vec_t * v);

/*
 * Appends a new hocis_vec_item_t to the vector, setting it as non-free.
 *
 * Returns a pointer to the data with the agreed constant available size.
 *
 * The data is memset to 0, user is expected to further initialize it.
 * The data is freed when freeing entire vector.
 *
 * Abstractly similar to how malloc works, but "managed"
 */
void * hocis_vec_append(hocis_vec_t * v);
void hocis_vec_remove(hocis_vec_t * v, hocis_vec_item_t * item);

hocis_vec_item_t * hocis_vec_get_from_ind(hocis_vec_t * v, size_t index);
hocis_vec_item_t * hocis_vec_get_from_data(hocis_vec_t * v, void * data);
size_t hocis_vec_get_ind(hocis_vec_t * v, hocis_vec_item_t * item);

void * hocis_vec_data_from_item(hocis_vec_item_t * i);

void __hocis_vec_resize(hocis_vec_t * v, size_t new_item_count);

#endif

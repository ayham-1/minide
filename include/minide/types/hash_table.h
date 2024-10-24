#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

typedef uint64_t (*hash_function)(const uint8_t * const key);
typedef bool (*eql_function)(const uint8_t * const key1, const uint8_t * const key2);

typedef struct hash_table_entry_t hash_table_entry_t;
typedef hash_table_entry_t * hash_table_iter_t;

struct hash_table_entry_t {
	hash_table_entry_t * prev;
	hash_table_entry_t * next;

	uint8_t * key;
	uint8_t * data;
};

typedef bool (*hash_table_entry_cleanup)(hash_table_entry_t * entry);

typedef struct {
	hash_function hashFunc;
	eql_function eqlFunc;
	hash_table_entry_cleanup entryCleanupFunc;

	size_t capacity;

	hash_table_entry_t ** buckets;

	size_t collisions;
} hash_table_t;

bool hash_table_create(hash_table_t * const table, size_t capacity, hash_function hashFunc, eql_function eqlFunc,
		       hash_table_entry_cleanup entryCleanupFunc);
void hash_table_cleanup(hash_table_t * const table);

bool hash_table_insert(hash_table_t * const table, uint8_t * const key, uint8_t * const data);

bool hash_table_get(const hash_table_t * const table, const uint8_t * const key, hash_table_entry_t ** out);
bool hash_table_remove(hash_table_t * const table, const uint8_t * const key);

///// DEBUG

typedef void (*hash_table_entry_printer)(const hash_table_entry_t * const entry);

void hash_table_debug(hash_table_t * table, hash_table_entry_printer printer);

#endif

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#include "../logger.h"

typedef uint64_t (*hash_function)(uint8_t* key);

typedef struct hash_table_entry_t hash_table_entry_t;

struct hash_table_entry_t {
    hash_table_entry_t* prev;
    hash_table_entry_t* next;

    uint8_t* key;
    uint8_t* data;
};

typedef bool (*hash_table_entry_cleanup)(hash_table_entry_t* entry);

typedef struct {
    hash_function hashFunc;
    hash_table_entry_cleanup entryCleanupFunc;

    size_t capacity;
    size_t bucketDepth;
    bool enforceBucketDepth;

    hash_table_entry_t** buckets;

    size_t collisions;
} hash_table_t;

bool hash_table_create(hash_table_t* table,
                       size_t capacity, size_t bucketDepth,
                       bool enforceBucketDepth,
                       hash_function hashFunc,
                       hash_table_entry_cleanup entryCleanupFunc);
void hash_table_cleanup(hash_table_t* table);

bool hash_table_insert(hash_table_t* table, 
                       uint8_t *key, uint8_t* data);

bool hash_table_get(hash_table_t* table,
                    uint8_t* key, uint8_t* out);

bool hash_table_remove(hash_table_t* table,
                       uint8_t *key);

#endif

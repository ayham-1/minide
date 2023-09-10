#include "hash_table.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

bool hash_table_create(hash_table_t* table,
                       size_t capacity, size_t bucketDepth,
                       bool enforceBucketDepth,
                       hash_function hashFunc,
                       hash_table_entry_cleanup entryCleanupFunc) {
    table->hashFunc = hashFunc;
    table->entryCleanupFunc = entryCleanupFunc;

    table->capacity = capacity;
    table->bucketDepth = bucketDepth;
    table->enforceBucketDepth = enforceBucketDepth;

    table->buckets = malloc(capacity * 
                            sizeof(hash_table_entry_t*));
    memset(table->buckets, 0x0, 
           capacity * sizeof(hash_table_entry_t*));
    table->collisions = 0;

    log_info("initialized new hash table with dimensions of %ix%i, depth enforcable: %i",
             table->capacity, table->bucketDepth, table->enforceBucketDepth);
    log_info("new hash table current size is %i bytes", 
             capacity * sizeof(hash_table_entry_t*));

    return true;
}
void hash_table_cleanup(hash_table_t* table) {
    for (size_t i = 0; i < table->capacity; i++) {
        if (table->buckets[i] == NULL) continue;

        hash_table_entry_t* curr_entry = table->buckets[i];

        hash_table_entry_t* start_entry = curr_entry;

        while(curr_entry != NULL) {
            // TODO(ayham): gracefully tell user of issue.
            assert(table->entryCleanupFunc(curr_entry));
            curr_entry = curr_entry->next;
        }

        curr_entry = table->buckets[i];
        while(curr_entry != NULL) {
            hash_table_entry_t* next = curr_entry->next;
            free(curr_entry);
            curr_entry = next;
        }
    }

    free(table->buckets);
}

bool hash_table_insert(hash_table_t* table, 
                       uint8_t *key, uint8_t* data) {
    uint64_t hash_value = table->hashFunc(key);
    uint64_t index = hash_value % (table->capacity - 1);

    if (table->buckets[index] != 0) {
        table->collisions++;
    }

    hash_table_entry_t* final_entry = NULL;

    if (table->buckets[index] == NULL) {
        table->buckets[index] = malloc(sizeof(hash_table_entry_t));
        final_entry = table->buckets[index];
        final_entry->prev = NULL;
        final_entry->next = NULL;
    } else { // collision
        hash_table_entry_t* deepest_bucket_entry = 
            table->buckets[index];
        while(deepest_bucket_entry->next != NULL)
            deepest_bucket_entry = deepest_bucket_entry->next;

        deepest_bucket_entry->next = malloc(sizeof(hash_table_entry_t));
        final_entry = deepest_bucket_entry->next;
        final_entry->prev = deepest_bucket_entry;
        final_entry->next = NULL;
    }

    final_entry->key = key;
    final_entry->data = data;

    return true;
}

bool hash_table_remove(hash_table_t* table,
                       uint8_t *key);

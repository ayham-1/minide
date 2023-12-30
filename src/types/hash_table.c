#include "minide/types/hash_table.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

bool hash_table_create(hash_table_t *table, size_t capacity,
                       hash_function hashFunc, eql_function eqlFunc,
                       hash_table_entry_cleanup entryCleanupFunc) {
  table->hashFunc = hashFunc;
  table->eqlFunc = eqlFunc;
  table->entryCleanupFunc = entryCleanupFunc;

  table->capacity = capacity;

  table->buckets = malloc(capacity * sizeof(hash_table_entry_t *));
  memset(table->buckets, 0x0, capacity * sizeof(hash_table_entry_t *));
  table->collisions = 0;

  log_info("initialized new hash table with %i buckets", table->capacity);
  log_info("new hash table current size is %i bytes",
           capacity * sizeof(hash_table_entry_t *));

  return true;
}
void hash_table_cleanup(hash_table_t *table) {
  for (size_t i = 0; i < table->capacity; i++) {
    if (table->buckets[i] == NULL)
      continue;

    hash_table_entry_t *curr_entry = table->buckets[i];

    while (curr_entry != NULL) {
      if (!table->entryCleanupFunc(curr_entry))
        log_error("failed cleanup of curr_entry, memory leaks could occur!");
      curr_entry = curr_entry->next;
    }

    curr_entry = table->buckets[i];
    while (curr_entry != NULL) {
      hash_table_entry_t *next = curr_entry->next;
      free(curr_entry);
      curr_entry = next;
    }
  }

  free(table->buckets);
}

bool hash_table_insert(hash_table_t *const table, uint8_t *const key,
                       uint8_t *const data) {
  uint64_t hash_value = table->hashFunc(key);
  uint64_t index = hash_value % (table->capacity - 1);

  if (table->buckets[index] != 0) {
    table->collisions++;
  }

  hash_table_entry_t *final_entry = NULL;

  if (table->buckets[index] == NULL) {
    table->buckets[index] = malloc(sizeof(hash_table_entry_t));
    final_entry = table->buckets[index];
    final_entry->prev = NULL;
    final_entry->next = NULL;
  } else { // collision
    hash_table_entry_t *new_entry = malloc(sizeof(hash_table_entry_t));
    final_entry = new_entry;
    final_entry->prev = NULL;
    final_entry->next = table->buckets[index];
    table->buckets[index]->prev = final_entry;
  }

  final_entry->key = key;
  final_entry->data = data;
  table->buckets[index] = final_entry;

  return true;
}

bool hash_table_get(const hash_table_t *const table, const uint8_t *key,
                    hash_table_entry_t **out) {
  uint64_t hash_value = table->hashFunc(key);
  uint64_t index = hash_value % (table->capacity - 1);

  if (table->buckets[index] == NULL)
    return false;

  hash_table_entry_t *curr_entry = table->buckets[index];
  while (curr_entry->next != NULL && !table->eqlFunc(curr_entry->key, key))
    curr_entry = curr_entry->next;

  if (!table->eqlFunc(curr_entry->key, key))
    return false;

  *out = curr_entry;
  return true;
}

bool hash_table_remove(hash_table_t *const table, const uint8_t *const key) {
  uint64_t hash_value = table->hashFunc(key);
  uint64_t index = hash_value % (table->capacity - 1);

  if (table->buckets[index] == NULL)
    return false;

  hash_table_entry_t *curr_entry = table->buckets[index];
  while (curr_entry->next != NULL && !table->eqlFunc(curr_entry->key, key))
    curr_entry = curr_entry->next;

  if (!table->eqlFunc(curr_entry->key, key))
    return false;

  curr_entry->prev->next = curr_entry->next;
  curr_entry->next->prev = curr_entry->prev;

  table->entryCleanupFunc(curr_entry);

  return true;
}

void hash_table_debug(hash_table_t *table, hash_table_entry_printer printer) {
  log_debug("================================");
  log_debug("printing hashtable of size %i",
            table->capacity * sizeof(hash_table_entry_t *));
  log_debug("capacity: %i", table->capacity);
  log_debug("collisions: %i", table->collisions);

  size_t printed = 0;
  for (size_t i = 0; i < table->capacity; i++) {
    if (!table->buckets[i])
      continue;
    printed++;
    printer(table->buckets[i]);
  }

  log_debug("entries printed: %i", printed);
  log_debug("================================");
}

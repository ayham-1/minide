#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../src/types/hash_table.h"
#include "../src/logger.h"

uint64_t hash(const uint8_t* const key) {
    return 1; // force hash collision
}

bool cleanup(hash_table_entry_t* entry) {
    return true;
}

bool eql(const uint8_t* const key1, const uint8_t* const key2) {
    return *key1 == *key2;
}

int main(int argc, char *argv[]) {
    logger_init(DEBUG, "", false);

    hash_table_t table;
    hash_table_create(&table, 100, 5, false, hash, eql, cleanup);

    log_info("hash collisions: %i", table.collisions);

    uint8_t key = 'A';
    uint8_t data = 'B';
    hash_table_insert(&table, &key, &data);

    log_info("hash collisions: %i", table.collisions);

    hash_table_entry_t* entry = NULL;
    assert(hash_table_get(&table, &key, &entry));
    assert(entry->key == &key);
    assert(entry->data == &data);

    assert(!hash_table_get(&table, &data, &entry));
    key = 'B';
    data = 'C';
    hash_table_insert(&table, &key, &data);
    entry = NULL;
    assert(hash_table_get(&table, &key, &entry));
    assert(entry->key == &key);
    assert(entry->data == &data);

    hash_table_cleanup(&table);
    logger_cleanup();
    return 0;
}

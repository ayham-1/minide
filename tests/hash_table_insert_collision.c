#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/logger.h"
#include "../src/types/hash_table.h"

uint64_t hash(const uint8_t *const key) { return 1; }

bool cleanup(hash_table_entry_t *entry) { return true; }

bool eql_entry(const uint8_t *const key1, const uint8_t *const key2) {
    return *key1 == *key2;
}

int main(int argc, char *argv[]) {
    logger_init(DEBUG, "", false);

    hash_table_t table;
    hash_table_create(&table, 100, hash, eql_entry, cleanup);

    log_info("hash collisions: %i", table.collisions);

    uint8_t key = 'A';
    uint8_t data = 'B';
    hash_table_insert(&table, &key, &data);

    log_info("hash collisions: %i", table.collisions);

    uint8_t key2 = 'B';
    uint8_t data2 = 'A';
    hash_table_insert(&table, &key2, &data2);

    log_info("hash collisions: %i", table.collisions);
    assert(table.collisions == 1);

    hash_table_cleanup(&table);
    logger_cleanup();
    return 0;
}

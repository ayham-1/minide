#ifndef PAIR_H
#define PAIR_H

#include <stdint.h>

typedef struct {
    uint8_t* key;
    uint8_t* data;
} pair_t;

#endif

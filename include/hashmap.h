#ifndef RUNA_HASHMAP_H
#define RUNA_HASHMAP_H

#include "vector.h"

typedef struct runa_hashmap_data {
    struct runa_tables* keys;
    struct runa_tables* values;
} runa_hashmap_data;

typedef runa_vector runa_hashmap;

#endif

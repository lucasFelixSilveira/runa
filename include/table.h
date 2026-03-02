#ifndef RUNA_TABLE_H
#define RUNA_TABLE_H

#include "hashmap.h"

#define runa_false 0
#define runa_true !0

typedef enum runa_table_kind {
    runa_integer,
    runa_string,
    runa_boolean,
    runa_nil,
    runa_map,
    runa_function
} runa_table_kind;

typedef struct runa_table {
    char local;
    runa_table_kind kind;
    union {
        int integer;
        char* string;
        char boolean;
        void* nil;
        runa_hashmap *hashmap;
    } value;
} runa_table;

runa_table* runa_table_new(runa_table_kind kind, void *value, char local);

#endif

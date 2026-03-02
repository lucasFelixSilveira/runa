#ifndef RUNA_H
#define RUNA_H

#include "table.h"
#include <stdio.h>

typedef void (*RunaCallback)(void *runa);

typedef struct Runa {
    FILE *file;
    runa_table *result;
} Runa;

Runa *runa_new(char *filename);

void runa_push_result(Runa *runa, runa_table *table);
runa_table *runa_pop_result(Runa *runa);

#endif

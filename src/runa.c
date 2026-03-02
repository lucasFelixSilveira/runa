#include <stdio.h>
#include <stdlib.h>
#include "runa.h"
#include "table.h"

void runa_print(void *runa) {
    Runa *data = (Runa*)runa;
    // char *arguments =
}

Runa *runa_new(char *filename) {
    Runa *runa = (Runa*)malloc(sizeof(Runa));
    runa->result = runa_table_new(runa_integer, 0, 0);
    runa->file = fopen(filename, "r+");
    return runa;
}

void runa_push_result(Runa *runa, runa_table *table) {
    free(runa->result);
    runa->result = table;
}

runa_table *runa_pop_result(Runa *runa) {
    return runa->result;
}

#include "table.h"
#include "runa.h"

#include <stdlib.h>
#include <string.h>

void runa_access(Runa *runa, runa_table *x, runa_table *y);
void runa_concatenate(Runa *runa, runa_table *x, runa_table *y);

void runa_add(Runa *runa, runa_table *x, runa_table *y);
void runa_sub(Runa *runa, runa_table *x, runa_table *y);
void runa_div(Runa *runa, runa_table *x, runa_table *y);
void runa_mul(Runa *runa, runa_table *x, runa_table *y);
void runa_mod(Runa *runa, runa_table *x, runa_table *y);

void runa_equals(Runa *runa, runa_table *x, runa_table *y) {
    if( x->kind != y->kind ) {
        runa_push_result(runa, runa_table_new(runa_boolean, runa_false, 1));
        return;
    };

    char *are = (char*)malloc(sizeof(char));
    if( x->kind == runa_string  ) *are = strcmp(x->value.string, y->value.string) == 0 ? 1 : 0;
    if( x->kind == runa_integer ) *are = (x->value.integer == y->value.integer) ? 1 : 0;
    if( x->kind == runa_boolean ) *are = (x->value.boolean == y->value.boolean) ? 1 : 0;

    runa_push_result(runa, runa_table_new(runa_boolean, are, 1));
}

void runa_different(Runa *runa, runa_table *x, runa_table *y) {
    runa_equals(runa, x, y);
    runa_table *result = runa_pop_result(runa);
    char *addr = (char*)malloc(1);
    *addr = !result->value.boolean;
    runa_push_result(runa, runa_table_new(runa_boolean, addr, 1));
}

void runa_more_than(Runa *runa, runa_table *x, runa_table *y);
void runa_less_than(Runa *runa, runa_table *x, runa_table *y);

void runa_more_than_or_equal(Runa *runa, runa_table *x, runa_table *y);
void runa_less_than_or_equal(Runa *runa, runa_table *x, runa_table *y);

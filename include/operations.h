#ifndef RUNA_OPERATIONS_H
#define RUNA_OPERATIONS_H
#include "table.h"
#include "runa.h"

void runa_access(Runa *runa, runa_table *x, runa_table *y);
void runa_concatenate(Runa *runa, runa_table *x, runa_table *y);

void runa_add(Runa *runa, runa_table *x, runa_table *y);
void runa_sub(Runa *runa, runa_table *x, runa_table *y);
void runa_div(Runa *runa, runa_table *x, runa_table *y);
void runa_mul(Runa *runa, runa_table *x, runa_table *y);
void runa_mod(Runa *runa, runa_table *x, runa_table *y);

void runa_equals(Runa *runa, runa_table *x, runa_table *y);
void runa_different(Runa *runa, runa_table *x, runa_table *y);

void runa_more_than(Runa *runa, runa_table *x, runa_table *y);
void runa_less_than(Runa *runa, runa_table *x, runa_table *y);

void runa_more_than_or_equal(Runa *runa, runa_table *x, runa_table *y);
void runa_less_than_or_equal(Runa *runa, runa_table *x, runa_table *y);

#endif

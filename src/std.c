#include "runa.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

void runa_print(Runa *runa) {
    runa_value **args = runa_stack_peek(runa->arguments);
    char *data = runa_value_to_string(args[0]);
    puts(data);
    free(data);
}

void runa_clear(Runa *runa) {
    runa_value **args = runa_stack_peek(runa->arguments);
    runa_value *value = args[0];
    runa_value_free(value, false);
}

void runa_mlog2(Runa *runa) {
    runa_value **args = runa_stack_peek(runa->arguments);
    runa_value *value = args[0];

    int bytes = value->value.integer;
    int bits = bytes * 8;

    runa_value *result = malloc(sizeof(runa_value));
    result->kind = runa_integer;
    result->value.integer = ((int)log2(bits)) - 2;
    runa_push_result(runa, result);
}

#include "runa.h"
#include <stdio.h>
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

#include "runa.h"
#include <stdio.h>
#include <stdlib.h>

void runa_print(Runa *runa) {
    runa_value **args = runa_stack_peek(runa->arguments);
    char *data = runa_value_to_string(args[0]);
    puts(data);
    free(data);
}

void runa_public(Runa *runa) {
    runa_value **args = runa_stack_peek(runa->arguments);
    char *public = runa_value_to_string(args[0]);
    int debug = args[1]->value.integer;
    if( args[1]->kind == runa_integer && args[1]->value.integer ) {
        printf("[RUNA] `%s` value made public argument kind %d\n", public, args[1]->kind);
    }
    free(public);
}

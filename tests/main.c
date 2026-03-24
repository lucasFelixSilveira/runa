#include "runa.h"
#include <math.h>
#include <stdio.h>

void mlog2(Runa *runa) {
    RunaValueFFI val = runa_peek_arg(runa, 0);
    if( val.tag != runa_integer ) return;
    runa_push_result(runa, (RunaValueFFI) {
        .tag = runa_integer,
        .data.integer = log2(val.data.integer) - 2
    });
}

void print(Runa *runa) {
    RunaValueFFI val = runa_peek_arg(runa, 0);
    char *str = runa_value_to_string(val);
    printf("print: %s\n", str);
    runa_optional(RUNA_FREE_STRING_BY_VALUE, runa_str_free, str, val);
    runa_value_free(val);
}

int main() {
    Runa *runa = runa_start();
    runa_push_function(runa, "print", (runa_callback)print, 1);
    runa_push_function(runa, "mlog2", (runa_callback)mlog2, 1);
    runa_loadfile(runa, "./main.lua");
    runa_free(runa);
    return 0;
}

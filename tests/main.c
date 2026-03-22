#include "runa.h"
#include <stdio.h>

void print(Runa *runa) {
    RunaValueFFI val = runa_peek_arg(runa, 0);
    printf("print: %s\n", val.data.string);
    runa_value_free(val);
}

int main() {
    Runa *runa = runa_start();
    runa_push_function(runa, "print", (runa_callback)print, 1);
    runa_loadfile(runa, "./main.lua");
    runa_free(runa);
    return 0;
}

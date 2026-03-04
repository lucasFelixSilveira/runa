#include "runa.h"

#include <stdio.h>
#include <stdlib.h>

void test_function(Runa *runa) {
    printf("hello!\n");
}

int main() {
    Runa *runa = malloc(sizeof(Runa));
    runa_push_function(runa, "hello", (runa_callback)test_function, 0);
    runa_loadfile(runa, "main.lua");
}

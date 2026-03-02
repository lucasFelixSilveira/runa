#include "runa.h"
#include "table.h"
#include "operations.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
    Runa *runa = runa_new("test.lua");
    printf("Result: %d", runa_pop_result(runa)->value.boolean);
}

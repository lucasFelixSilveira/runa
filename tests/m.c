#include "runa.h"
#include <stdlib.h>

int main() {
    Runa *runa = malloc(sizeof(Runa));
    runa_start(runa);
    runa_use_std(runa, COMMON_STD | MORGANA_STD);
    runa_loadfile(runa, "main.lua");
    runa_free(runa);
}

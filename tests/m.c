#include "runa.h"
#include <stdlib.h>

int main() {
    Runa *runa = malloc(sizeof(Runa));
    runa_use_std(runa);
    runa_loadfile(runa, "main.lua");
}

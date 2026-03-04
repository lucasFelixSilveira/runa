#include "runa.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
    Runa *runa = malloc(sizeof(Runa));
    runa_loadfile(runa, "main.lua");
}

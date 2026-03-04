#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "runa.h"

void runa_print(void *runa) {
    Runa *data = (Runa*)runa;
}

void runa_loadfile(Runa *runa, char *filename) {
    runa->file = fopen(filename, "r+");
    runa_parse(runa);
}

bool runa_send_error(Runa *runa, runa_error error, char *what) {
    runa->error = true;
    switch(error) {
        case RUNA_IS_NOT_A_FUNCTION: {
            printf("%s isn't a valid function.\n", what);
        } break;
    }

    return true;
}

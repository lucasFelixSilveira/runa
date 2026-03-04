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

void runa_push_function(Runa *runa, char *id, runa_callback cb, int argc) {
    runa->functions = realloc(runa->functions, sizeof(runa_function) * (runa->functions_length + 1));
    runa->functions[runa->functions_length++] = (runa_function) {
        .function = cb,
        .arguments = argc,
        .identifier = id
    };
}

bool runa_send_error(Runa *runa, runa_error error, char *what) {
    runa->error = true;
    switch(error) {
        case RUNA_IS_NOT_A_FUNCTION: {
            printf("%s isn't a valid function.\n", what);
        } break;

        case RUNA_ARGUMENTS_COUNT_WRONG: {
            printf("the function %s arguments count is wrong.\n", what);
        } break;
    }

    return true;
}

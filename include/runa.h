#ifndef RUNA_H
#define RUNA_H

#include <stdbool.h>
#include <stdio.h>



typedef void (*runa_callback)(void *runa);

typedef struct runa_functions {
    char *identifier;
    runa_callback function;
} runa_functions;

typedef struct Runa {
    FILE *file;
    bool error;
    int functions_length;
    runa_functions *functions;
} Runa;

void runa_loadfile(Runa *runa, char *filename);

typedef enum runa_error {
    RUNA_IS_NOT_A_FUNCTION,
} runa_error;

bool runa_send_error(Runa *runa, runa_error error, char *what);

#endif

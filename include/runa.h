#ifndef RUNA_H
#define RUNA_H

#include <stdbool.h>
#include <stdio.h>

typedef void (*runa_callback)(void *runa);

typedef struct runa_functions {
    char *identifier;
    int arguments;
    runa_callback function;
} runa_function;

typedef struct Runa {
    FILE *file;
    bool error;
    int functions_length;
    runa_function *functions;
} Runa;

void runa_loadfile(Runa *runa, char *filename);
void runa_push_function(Runa *runa, char *identifier, runa_callback cb, int argc);

typedef enum runa_error {
    RUNA_IS_NOT_A_FUNCTION,
    RUNA_ARGUMENTS_COUNT_WRONG
} runa_error;

bool runa_send_error(Runa *runa, runa_error error, char *what);

#endif

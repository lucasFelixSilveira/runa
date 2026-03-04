#ifndef RUNA_H
#define RUNA_H

#include "stack.h"
#include <stdbool.h>
#include <stdio.h>

typedef void (*runa_callback)(void *runa);

typedef enum runa_value_kind {
    runa_string,
    runa_integer
} runa_value_kind;

typedef struct runa_value {
    runa_value_kind kind;
    union {
        char *string;
        int integer;
    } value;
} runa_value;

typedef struct runa_functions {
    char *identifier;
    int arguments;
    runa_callback function;
} runa_function;

typedef struct Runa {
    FILE *file;
    bool error;
    runa_stack *arguments;
    int functions_length;
    runa_function *functions;
} Runa;

void runa_loadfile(Runa *runa, char *filename);
void runa_push_function(Runa *runa, char *identifier, runa_callback cb, int argc);
void runa_use_std(Runa *runa);

typedef enum runa_error {
    RUNA_IS_NOT_A_FUNCTION,
    RUNA_ARGUMENTS_COUNT_WRONG,
    RUNA_INVALID_SYNTAX_IN_CALL
} runa_error;

bool runa_send_error(Runa *runa, runa_error error, char *what);
bool runa_send_error_type(Runa *runa, char *where, char *expected, char *received);
char *runa_value_kind_str(runa_value_kind kind);

#endif

#ifndef RUNA_H
#define RUNA_H

#include "stack.h"
#include <stdbool.h>
#include <stdio.h>

typedef void (*runa_callback)(void *runa);

typedef enum runa_value_kind {
    runa_string,
    runa_integer,
    runa_float,
    runa_nil
} runa_value_kind;

typedef struct runa_value {
    runa_value_kind kind;
    union {
        char *string;
        int integer;
        long double _float;
        void *nil;
    } value;
} runa_value;

typedef struct runa_function {
    char *identifier;
    int arguments;
    runa_callback function;
} runa_function;

typedef struct runa_local {
    char *identifier;
    runa_value *value;
} runa_local;

typedef struct runa_locals {
    int length;
    runa_local *values;
} runa_locals;

typedef struct Runa {
    FILE *file;
    bool error;
    runa_stack *arguments;
    int functions_length;
    runa_function *functions;
    runa_stack *stack_locals;
    runa_locals locals;
    runa_value *result;
} Runa;

void runa_loadfile(Runa *runa, char *filename);
void runa_push_function(Runa *runa, char *identifier, runa_callback cb, int argc);

void runa_push_result(Runa *runa, runa_value *value);

void runa_push_local(Runa *runa, char *id, runa_value *value);
bool runa_peek_local(Runa *runa, char *id, runa_value **value);
void runa_assign_local(Runa *runa, char *id, runa_value *value);

void runa_use_std(Runa *runa);

typedef enum runa_error {
    RUNA_IS_NOT_A_FUNCTION,
    RUNA_ARGUMENTS_COUNT_WRONG,
    RUNA_INVALID_SYNTAX_IN_CALL,
    RUNA_INVALID_SYNTAX_IN_LOCAL,
    RUNA_UNKNOWN_SYMBOL,
    RUNA_INVALID_SYNTAX_OF_EXPRESSION,
} runa_error;

bool runa_send_error(Runa *runa, runa_error error, char *what);
bool runa_send_error_type(Runa *runa, char *where, char *expected, char *received);
char *runa_value_to_string(runa_value *value);
char *runa_value_kind_str(runa_value_kind kind);

#endif

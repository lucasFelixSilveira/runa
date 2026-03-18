#ifndef RUNA_H
#define RUNA_H

#include "stack.h"
#include <stdbool.h>
#include <stdio.h>

#define FLAG_FUNCTION_CALLED (1 << 0)

#define MOD_NOT (1 << 0)
#define MOD_AND (1 << 1)
#define MOD_OR  (1 << 2)

#define RUNA_VALUE_KIND_FIELDS \
    X(runa_string,  "string") \
    X(runa_integer, "integer") \
    X(runa_float,   "float") \
    X(runa_table,   "table") \
    X(runa_nil,     "nil")

#define RUNA_ERROR_FIELDS \
    X(RUNA_OUT_OF_MEMORY,                             "%s is out of memory") \
    X(RUNA_IS_NOT_A_FUNCTION,                         "%s isn't a valid function.") \
    X(RUNA_ARGUMENTS_COUNT_WRONG,                     "the function %s arguments count is wrong.") \
    X(RUNA_INVALID_SYNTAX_IN_CALL,                    "the syntax call of %s is wrong.") \
    X(RUNA_INVALID_SYNTAX_IN_LOCAL,                   "the syntax local of %s is wrong.") \
    X(RUNA_UNKNOWN_SYMBOL,                            "the symbol %s is unknown.") \
    X(RUNA_INVALID_SYNTAX_OF_EXPRESSION,              "invalid syntax of expression. `%s` was the reason.") \
    X(RUNA_ACCESS_INVALID_BECAUSE_IDENTIFIER,         "you can't use access opration in %s. Because it isn't a table.") \
    X(RUNA_TABLES_CANT_DO_NOTHING_EXCEPT_CONCATENATE, "the `%s` value is a Table, and tables can't do no one operation except concatenate.") \
    X(RUNA_TABLE_FIELD_INVALID,                       "The `%s` table field was not found.") \
    X(RUNA_UNMATCHED_END,                             "unmatched end %s") \
    X(RUNA_INVALID_SYNTAX,                            "invalid syntax. `%s` was the reason.") \
    X(RUNA_COMPRESSION_FAILED,                        "Runa'd try compress the body of the function to more memory efficiency. But the compression was failed. %s")

#define X(kind, name) kind,
typedef enum runa_value_kind { RUNA_VALUE_KIND_FIELDS } runa_value_kind;
#undef X

#define X(kind, name) kind,
typedef enum runa_error { RUNA_ERROR_FIELDS } runa_error;
#undef X

typedef enum std_flags {
    COMMON_STD  = 1 << 1,
    MORGANA_STD = 1 << 2,
} std_flags;

typedef void (*runa_callback)(void *runa);

typedef struct runa_table_field {
    char *identifier;
    void *value;
} runa_table_field;

typedef struct runa_value {
    runa_value_kind kind;
    union {
        char *string;
        int integer;
        long double _float;
        void *nil;
        void *table;
    } value;
} runa_value;

typedef struct {
    int flags;
    runa_value *result;
    int mod;
    bool state;
} call_state;

typedef struct runa_function {
    char **params;
    int arguments;

    char *identifier;

    // builtin
    runa_callback function;

    // user-defined
    char *body;
    int body_size;
} runa_function;

typedef struct runa_local {
    char *identifier;
    runa_value *value;
} runa_local;

typedef struct runa_locals {
    int length;
    runa_local *values;
} runa_locals;

typedef struct runa_frame {
    int last_did, flags, mod;
    bool state, should_leave;
} runa_frame;

typedef struct Runa {
    FILE *file;
    char *pushed;
    bool error;
    runa_stack *arguments;
    int functions_length;
    runa_function *functions;
    runa_stack *stack_locals;
    runa_locals *locals;
    runa_value *result;
    runa_stack *if_stack;
    runa_stack *code_stack;
    runa_stack *frames;

    int last_did, flags, mod;
    bool state, should_leave;
} Runa;

void runa_start(Runa *runa);
void runa_free(Runa *runa);
void runa_loadfile(Runa *runa, char *filename);
void runa_push_function(Runa *runa, char *identifier, runa_callback cb, int argc);

void runa_push_result(Runa *runa, runa_value *value);

void runa_push_local(Runa *runa, char *id, runa_value *value);
bool runa_peek_local(Runa *runa, char *id, runa_value **value);
void runa_assign_local(Runa *runa, char *id, runa_value *value);
void runa_value_free(runa_value *value, bool real);

void runa_use_std(Runa *runa, int flags);

bool runa_send_error(Runa *runa, runa_error error, char *what);
bool runa_send_fatal_error(Runa *runa, runa_error error, char *what);
bool runa_send_error_type(Runa *runa, char *where, char *expected, char *received);

char *runa_value_to_string(runa_value *value);
char *runa_value_kind_str(runa_value_kind kind);

runa_value *runa_access_table(runa_value *table, char *str);

#endif

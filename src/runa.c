#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "stack.h"
#include "runa.h"
#include "std.h"

void runa_loadfile(Runa *runa, char *filename) {
    runa->file = fopen(filename, "r+");
    runa->arguments = runa_stack_new(64);
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

char *runa_value_kind_str(runa_value_kind kind) {
    switch(kind) {
        case runa_string: return "string";
        case runa_integer: return "integer";
    }
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

        case RUNA_INVALID_SYNTAX_IN_CALL: {
            printf("The syntax call of %s is wrong.\n", what);
        } break;
    }

    return true;
}

bool runa_send_error_type(Runa *runa, char *where, char *expected, char *received) {
    runa->error = true;
    printf("Has been expected %s in %s, but was found %s.\n", expected, where, received);
    return true;
}

void runa_use_std(Runa *runa) {
    ___runa_use_std__(runa);
}

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "stack.h"
#include "runa.h"
#include "std.h"
#include "vector.h"

void runa_start(Runa *runa) {
    runa->functions_length = 0;
    runa->arguments = runa_stack_new(64);
    runa->stack_locals = runa_stack_new(128);
    runa->functions = malloc(0);
    runa->error = false;
    runa->locals.length = 0;
    runa->locals.values = malloc(0);
    runa->if_stack = runa_stack_new(64);
}

void runa_arguments_desconstructor(void *data) {
    runa_value **args = (runa_value**)data;
    if(! args ) return;
    free(args);
}

void runa_locals_desconstructor(void *data) {
    runa_value *value = (runa_value*)data;
    runa_value_free(value, true);
}

void runa_if_stack_desconstructor(void *data) {
    free(data);
}

void runa_free(Runa *runa) {
    for( int i = 0; i < runa->locals.length; i++ ) {
        runa_local local = runa->locals.values[i];
        runa_value_free(local.value, true);
        free(local.identifier);
        free(local.value);
    }
    free(runa->locals.values);
    runa_stack_free_all(runa->arguments, runa_arguments_desconstructor);
    runa_stack_free_all(runa->stack_locals, runa_locals_desconstructor);
    free(runa->functions);
    runa->functions_length = 0;
    runa_stack_free_all(runa->if_stack, NULL);
    free(runa);
}

void runa_loadfile(Runa *runa, char *filename) {
    runa->file = fopen(filename, "r+");
    runa_parse(runa);
    fclose(runa->file);
}

void runa_push_function(Runa *runa, char *id, runa_callback cb, int argc) {
    runa->functions = realloc(runa->functions, sizeof(runa_function) * (runa->functions_length + 1));
    runa->functions[runa->functions_length++] = (runa_function) {
        .function = cb,
        .arguments = argc,
        .identifier = id
    };
}

void runa_push_local(Runa *runa, char *id, runa_value *value) {
    runa->locals.values = realloc(runa->locals.values, sizeof(runa_local) * (runa->locals.length + 1));
    runa_value *value_copy = malloc(sizeof(runa_value));
    memcpy(value_copy, value, sizeof(runa_value));
    char *id_copy = strdup(id);
    runa->locals.values[runa->locals.length++] = (runa_local) {
        .identifier = id_copy,
        .value = value_copy
    };
}

void runa_push_result(Runa *runa, runa_value *value) {
    runa->result = value;
}

bool runa_peek_local(Runa *runa, char *id, runa_value **value) {
    for( int i = 0; i < runa->locals.length; i++ ) {
        runa_local *data = &runa->locals.values[i];
        if( data->identifier && strcmp(data->identifier, id) == 0 ) {
            if( data->value == NULL ) return false;
            *value = data->value;
            return true;
        }
    }

    *value = NULL;
    return false;
}

void runa_destroy_local(Runa *runa, char *id) {
    for( int i = 0; i < runa->locals.length; i++ ) {
        runa_local *data = &runa->locals.values[i];
        if( data->identifier && strcmp(data->identifier, id) == 0 ) {
            free(data->identifier);
            if( data->value != NULL ) {
                if( data->value->kind == runa_string && data->value->value.string != NULL ) free(data->value->value.string);
                free(data->value);
            }

            for( int j = i; j < runa->locals.length - 1; j++ )
            /* -> */ runa->locals.values[j] = runa->locals.values[j + 1];

            runa->locals.length--;
            if( runa->locals.length > 0 ) {
                runa->locals.values = realloc(runa->locals.values, sizeof(runa_local) * runa->locals.length);
                break;
            }

            free(runa->locals.values);
            runa->locals.values = NULL;
            break;
        }
    }
}

void runa_value_free(runa_value *value, bool real) {
    if( value->kind == runa_table ) {
        runa_vector *table = (runa_vector*)value->value.table;
        for( int i = 0; i < table->length; i++ ) {
            runa_table_field *field = runa_vector_get(table, i);
            free(field->identifier);
            runa_value_free(field->value, real);
            free(field->value);
            free(field);
        }
        if( real ) runa_vector_free(table);
    }

    if( value->kind == runa_string ) {
        free(value->value.string);
        if(! real ) {
            value->value.string = (char*)malloc(1);
            value->value.string[0] = 0;
        }
    }

    if( value->kind == runa_integer ) value->value.integer = 0;
    if( value->kind == runa_integer ) value->value._float = 0.0;
}

void runa_assign_local(Runa *runa, char *id, runa_value *value) {
    for( int i = 0; i < runa->locals.length; i++ ) {
        runa_local *data = &runa->locals.values[i];
        if( data->identifier && strcmp(data->identifier, id) == 0 ) {
            if( data->value != NULL ) {
                runa_value_free(data->value, true);
                free(data->value);
            }

            data->value = malloc(sizeof(runa_value));
            memcpy(data->value, value, sizeof(runa_value));
            if( value->kind == runa_string && value->value.string != NULL ) {
                data->value->value.string = malloc(strlen(value->value.string) + 1);
                strcpy(data->value->value.string, value->value.string);
            }
            return;
        }
    }

    runa_push_local(runa, id, value);
}

char *runa_value_to_string(runa_value *value) {
    if( value->kind == runa_string ) return strdup(value->value.string);

    if( value->kind == runa_integer ) {
        int digits = (value->value.integer == 0) ? 1 : (int)log10(value->value.integer < 0 ? -(double)value->value.integer : value->value.integer) + 1;
        char *data = (char*)malloc(digits);
        sprintf(data, "%d", value->value.integer);
        return data;
    }

    if(value->kind == runa_float) {
        int digits = snprintf(NULL, 0, "%Lg", value->value._float);
        char *data = malloc(digits + 1);
        snprintf(data, digits + 1, "%Lg", value->value._float);
        return data;
    }

    char *nil = (char*)malloc(4);
    memcpy(nil, "nil", 4);
    return nil;
}

char *runa_value_kind_str(runa_value_kind kind) {
    switch(kind) {
        case runa_string: return "string";
        case runa_integer: return "integer";
        case runa_float: return "float";
        case runa_nil: return "nil";
        case runa_table: return "table";
    }
}

runa_value *runa_access_table(runa_value *table, char *str) {
    runa_value *data = malloc(sizeof(runa_value));
    runa_vector *vec = (runa_vector*)table->value.table;
    for( int i = 0; i < vec->length; i++ ) {
        runa_table_field *field = runa_vector_get(vec, i);
        if( strcmp(field->identifier, str) == 0 ) {
            runa_value *val = (runa_value*)field->value;
            data->kind = val->kind;

            if( val->kind == runa_string ) {
                data->value.string = strdup(val->value.string);
                break;
            }

            data->value = val->value;
            break;
        }
    }
    return data;
}

bool runa_send_error(Runa *runa, runa_error error, char *what) {
    runa->error = true;
    switch(error) {
        case RUNA_OUT_OF_MEMORY: printf("%s is out of memory\n", what);
        break;

        case RUNA_IS_NOT_A_FUNCTION: printf("%s isn't a valid function.\n", what);
        break;

        case RUNA_ARGUMENTS_COUNT_WRONG: printf("the function %s arguments count is wrong.\n", what);
        break;

        case RUNA_INVALID_SYNTAX_IN_CALL: printf("The syntax call of %s is wrong.\n", what);
        break;

        case RUNA_INVALID_SYNTAX_IN_LOCAL: printf("The syntax local of %s is wrong.\n", what);
        break;

        case RUNA_UNKNOWN_SYMBOL: printf("The symbol %s is unknown.\n", what);
        break;

        case RUNA_INVALID_SYNTAX_OF_EXPRESSION: printf("Invalid syntax of expression. %s was the reason.\n", what);
        break;

        case RUNA_ACCESS_INVALID_BECAUSE_IDENTIFIER: printf("You can't use access opration in %s. Because it isn't a table.\n", what);
        break;

        case RUNA_TABLES_CANT_DO_NOTHING_EXCEPT_CONCATENATE: printf("The `%s` value is a Table, and tables can't do no one operation except concatenate.\n", what);
        break;

        case RUNA_TABLE_FIELD_INVALID: printf("The `%s` table field was not found.\n", what);
        break;

        case RUNA_UNMATCHED_END: printf("Unmatched end.\n");
        break;
    }

    return true;
}

bool runa_send_fatal_error(Runa *runa, runa_error error, char *what) {
    runa_send_error(runa, error, what);
    exit(1);
}

bool runa_send_error_type(Runa *runa, char *where, char *expected, char *received) {
    runa->error = true;
    printf("Has been expected %s in %s, but was found %s.\n", expected, where, received);
    return true;
}

void runa_use_std(Runa *runa, int flags) {
    if( flags & COMMON_STD ) ___runa_use_std__(runa);
    if( flags & MORGANA_STD ) ___runa_use_morgana_std__(runa);
}

#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "stack.h"
#include "runa.h"
#include "std.h"
#include "vector.h"

void runa_value_free(runa_value *value, bool real);

void printfln(char *format, ...) {
    va_list args;
    char *str = (char*)malloc(strlen(format) + 2);
    sprintf(str, "%s\n", format);
    va_start(args, format);
    vprintf(str, args);
    va_end(args);
    free(str);
}

void runa_start(Runa *runa) {
    runa->functions_length = 0;
    runa->arguments = runa_stack_new(64);
    runa->stack_locals = runa_stack_new(128);
    runa->functions = malloc(0);
    runa->error = false;
    runa->locals = malloc(sizeof(runa_locals));
    runa->locals->length = 0;
    runa->locals->values = malloc(0);
    runa->if_stack = runa_stack_new(64);
    runa->code_stack = runa_stack_new(64);
    runa->frames = runa_stack_new(64);

    runa->last_did = 0;
    runa->flags = 0;
    runa->mod = 0;
    runa->state = false;
    runa->should_leave = false;
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
    for( int i = 0; i < runa->locals->length; i++ ) {
        runa_local local = runa->locals->values[i];
        runa_value_free(local.value, true);
        free(local.identifier);
        free(local.value);
    }
    free(runa->locals->values);
    free(runa->locals);
    runa_stack_free_all(runa->arguments, runa_arguments_desconstructor);
    runa_stack_free_all(runa->stack_locals, runa_locals_desconstructor);
    free(runa->functions);
    runa->functions_length = 0;
    runa_stack_free_all(runa->if_stack, NULL);
    runa_stack_free_all(runa->code_stack, NULL);
    free(runa);
}

void runa_push_frame(Runa *runa) {
    runa_frame *frame = malloc(sizeof(runa_frame));
    frame->flags = runa->flags;
    frame->last_did = runa->last_did;
    frame->mod = runa->mod;
    frame->should_leave = runa->should_leave;
    frame->state = runa->state;

    runa_stack_push(runa->frames, frame);

    runa->flags = 0;
    runa->last_did = 0;
    runa->mod = 0;
    runa->should_leave = false;
    runa->state = false;
}

void runa_pop_frame(Runa *runa) {
    runa_frame *frame = runa_stack_pop(runa->frames);

    runa->last_did = frame->last_did;
    runa->flags = frame->flags;
    runa->mod = frame->mod;
    runa->state = frame->state;
    runa->should_leave = frame->should_leave;
}

void runa_push_scope(Runa *runa) {
    runa_locals *locals = malloc(sizeof(runa_locals));
    locals->length = runa->locals->length;
    locals->values = malloc(sizeof(runa_local) * locals->length);
    memcpy(locals->values, runa->locals->values, sizeof(runa_local) * locals->length);

    runa_stack_push(runa->stack_locals, locals);

    free(runa->locals->values);
    free(runa->locals);

    runa->locals = malloc(sizeof(runa_locals));
    runa->locals->length = 0;
    runa->locals->values = malloc(0);
}

void runa_pop_scope(Runa *runa) {
    for( int i = 0; i < runa->locals->length; i++ )
    /* -> */ runa_value_free(runa->locals->values[i].value, true);

    free(runa->locals->values);
    free(runa->locals);

    runa->locals = runa_stack_pop(runa->stack_locals);
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
    runa->locals->values = realloc(runa->locals->values, sizeof(runa_local) * (runa->locals->length + 1));
    runa_value *value_copy = malloc(sizeof(runa_value));
    memcpy(value_copy, value, sizeof(runa_value));
    char *id_copy = strdup(id);
    runa->locals->values[runa->locals->length++] = (runa_local) {
        .identifier = id_copy,
        .value = value_copy
    };
}

bool runa_peek_local(Runa *runa, char *id, runa_value **value) {
    for( int i = 0; i < runa->locals->length; i++ ) {
        runa_local *data = &runa->locals->values[i];
        if( data->identifier && strcmp(data->identifier, id) == 0 ) {
            *value = data->value;
            return true;
        }
    }

    for( int s = runa->stack_locals->length - 1; s >= 0; s-- ) {
        runa_locals *scope = runa->stack_locals->values[s];
        for( int i = 0; i < scope->length; i++ ) {
            runa_local *data = &scope->values[i];
            if( data->identifier && strcmp(data->identifier, id) == 0 ) {
                *value = data->value;
                return true;
            }
        }
    }

    *value = NULL;
    return false;
}

void runa_destroy_local(Runa *runa, char *id) {
    for( int i = 0; i < runa->locals->length; i++ ) {
        runa_local *data = &runa->locals->values[i];
        if( data->identifier && strcmp(data->identifier, id) == 0 ) {
            free(data->identifier);
            if( data->value != NULL ) {
                if( data->value->kind == runa_string && data->value->value.string != NULL ) free(data->value->value.string);
                free(data->value);
            }

            for( int j = i; j < runa->locals->length - 1; j++ )
            /* -> */ runa->locals->values[j] = runa->locals->values[j + 1];

            runa->locals->length--;
            if( runa->locals->length > 0 ) {
                runa->locals->values = realloc(runa->locals->values, sizeof(runa_local) * runa->locals->length);
                break;
            }

            free(runa->locals->values);
            runa->locals->values = NULL;
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
    if( value->kind == runa_float ) value->value._float = 0.0;
}

void runa_assign_local(Runa *runa, char *id, runa_value *value) {
    for( int i = 0; i < runa->locals->length; i++ ) {
        runa_local *data = &runa->locals->values[i];
        if( data->identifier && strcmp(data->identifier, id) == 0 ) goto assign_here;
    }

    for( int s = runa->stack_locals->length - 1; s >= 0; s-- ) {
        runa_locals *scope = runa->stack_locals->values[s];
        for( int i = 0; i < scope->length; i++ ) {
            runa_local *data = &scope->values[i];
            if( data->identifier && strcmp(data->identifier, id) == 0 ) {
                runa_value_free(data->value, true);
                free(data->value);

                data->value = malloc(sizeof(runa_value));
                memcpy(data->value, value, sizeof(runa_value));

                if( value->kind == runa_string && value->value.string != NULL ) data->value->value.string = strdup(value->value.string);
                return;
            }
        }
    }

    runa_push_local(runa, id, value);
    return;

assign_here:
    {
        runa_local *data = NULL;

        for( int i = 0; i < runa->locals->length; i++ ) {
            if( strcmp(runa->locals->values[i].identifier, id) == 0 ) {
                data = &runa->locals->values[i];
                break;
            }
        }

        if( data->value != NULL ) {
            runa_value_free(data->value, true);
            free(data->value);
        }

        data->value = malloc(sizeof(runa_value));
        memcpy(data->value, value, sizeof(runa_value));
        if( value->kind == runa_string && value->value.string != NULL ) data->value->value.string = strdup(value->value.string);
    }
}

void runa_push_result(Runa *runa, runa_value *value) {
    ((call_state*)runa_stack_peek(call_stack))->result = value;
}

char *runa_value_to_string(runa_value *value) {
    if( value->kind == runa_string ) return strdup(value->value.string);

    if( value->kind == runa_integer ) {
        int digits = (value->value.integer == 0) ? 1 : (int)log10(value->value.integer < 0 ? -(double)value->value.integer : value->value.integer) + 1;
        char *data = (char*)malloc(digits);
        sprintf(data, "%d", value->value.integer);
        return data;
    }

    if( value->kind == runa_float ) {
        int digits = snprintf(NULL, 0, "%Lg", value->value._float);
        char *data = (char*)malloc(digits + 1);
        snprintf(data, digits + 1, "%Lg", value->value._float);
        return data;
    }

    char *nil = (char*)malloc(4);
    memcpy(nil, "nil", 4);
    return nil;
}

char *runa_value_kind_str(runa_value_kind kind) {
    #define X(kind, name) case kind: return name;
    switch(kind) { RUNA_VALUE_KIND_FIELDS }
    #undef X
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
    #define X(kind, name) case kind: printfln(name, (what == NULL) ? "" : what); break;
    switch(error) { RUNA_ERROR_FIELDS };
    #undef X
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

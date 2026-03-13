#include "runa.h"
#include "lexer.h"
#include "parser.h"
#include "stack.h"
#include "checkout.h"
#include "expressions.h"
#include "statements.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int last_did = 0;
typedef enum last_did_flags {
    FUNCTION_CALLED = (1 << 0),
} last_did_flags;

bool check_if_is_function(Runa *runa, runa_function *function, char *identifier) {
    for( int i = 0; i < runa->functions_length; i++ ) {
        runa_function data = runa->functions[i];
        if( strcmp(data.identifier, identifier) == 0 ) {
            *function = data;
            return true;
        }
    }
    return false;
}

bool identifier(Runa *runa, char *token);

bool expression(Runa *runa, char *token, runa_value *value) {
    bool already_peeked = false;
    runa_value *peeked_value = NULL;

    if( isidentifier(token) ) {

        if( identifier(runa, token) ) {
            if( last_did & FUNCTION_CALLED ) {
                last_did &= ~FUNCTION_CALLED;
                runa_value *result = runa->result;
                memcpy(value, result, sizeof(runa_value));
                free(result);
                return true;
            }

            return runa_send_fatal_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, token);
        }

        char *operator = runa_token(runa);

        if( operator[0] == '[' ) {
            runa_value *table = &(runa_value){ .kind = runa_nil, .value.nil = NULL };
            runa_peek_local(runa, token, &table);

            if( table->kind != runa_table ) {
                free(operator);
                free(table);
                return runa_send_error(runa, RUNA_ACCESS_INVALID_BECAUSE_IDENTIFIER, token);
            }

            char *access = runa_token(runa);
            runa_value index = { .kind = runa_nil, .value.nil = NULL };
            if(! expression(runa, access, &index) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, access);

            char *str = runa_value_to_string(&index);
            runa_value *data = runa_access_table(table, str);
            if( isstring(access) ) free(index.value.string);
            free(access);

            if( data->kind == runa_nil ) return runa_send_fatal_error(runa, RUNA_TABLE_FIELD_INVALID, str);

            char *end = runa_token(runa);
            if( end[0] != ']' ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, end);

            free(operator);
            free(str);
            free(end);

            already_peeked = true;
            peeked_value = data;
            operator = runa_token(runa);
        }

        bool isoperation = false;

        checktoop(concat, "..");
        checktoop(add, "+");
        checktoop(sub, "-");
        checktoop(div, "/");
        checktoop(mul, "*");
        checktoop(mod, "%");
        checktoop(idiv, "//");

        if( (! already_peeked) && (! runa_peek_local(runa, token, &peeked_value)) ) return runa_send_error(runa, RUNA_UNKNOWN_SYMBOL, token);

        if( already_peeked && isoperation ) {
            if( peeked_value->kind == runa_string && (! isconcat) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, operator);

            char *ctx = runa_token(runa);
            runa_value rhs = {0};
            if(! expression(runa, ctx, &rhs) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, ctx);
            free(operator);

            runa_value *data = (runa_value*)malloc(sizeof(runa_value));
            data[0] = (runa_value) { .kind = runa_nil, .value.nil = NULL };
            if( isconcat && rhs.kind == runa_string ) {
                data->kind = runa_string;
                char *lhs = runa_value_to_string(peeked_value);
                int size = strlen(lhs) + strlen(rhs.value.string) + 1;
                data->value.string = (char*)malloc(size);
                sprintf(data->value.string, "%s%s", lhs, rhs.value.string);
                free(lhs);
                if( isstring(ctx) ) free(rhs.value.string);
                free(ctx);
                memcpy(value, data, sizeof(runa_value));
                free(data);
                return true;
            }
            else {
                free(ctx);
                return runa_send_error(runa, RUNA_TABLES_CANT_DO_NOTHING_EXCEPT_CONCATENATE, token);
            }
        }
        else
        if( isoperation ) {
            runa_back(runa, operator);
            if( peeked_value->kind == runa_string ) return string_expression(runa, token, value);
            if( peeked_value->kind == runa_float || peeked_value->kind == runa_integer ) return numeric_expression(runa, token, value);
        } else runa_back(runa, operator);

        memcpy(value, peeked_value, sizeof(runa_value));
        return true;
    }

    if( isnumber(token) ) return numeric_expression(runa, token, value);
    if( isstring(token) ) return string_expression(runa, token, value);

    if( strcmp(token, "{") == 0 ) return table_expression(runa, token, value);

    return false;
}

bool identifier(Runa *runa, char *token) {
    if(! isidentifier(token) ) return false;

    /* Check for all things who
     * starts with a identifier */

    char *next = runa_token(runa);
    if( next[0] == '(' ) {
        free(next);
        int o = position;

        runa_function function;
        if(! check_if_is_function(runa, &function, token) ) return runa_send_error(runa, RUNA_IS_NOT_A_FUNCTION, token);

        runa_value **args = (runa_value**)malloc(0);
        int argc = 0;
        while(1) {
            char *arg = runa_token(runa);
            if( arg[0] == ')' && argc == 0 ) {
                free(arg);
                break;
            };

            runa_value value = { .kind = runa_nil, .value.nil = NULL };
            if(! expression(runa, arg, &value) ) {
                free(arg);
                for( int i = 0; i < argc; i++ ) free(args[i]);
                free(args);
                return runa_send_error(runa, RUNA_INVALID_SYNTAX_IN_CALL, token);
            }
            free(arg);

            args = realloc(args, sizeof(runa_value*) * (argc + 1));
            args[argc] = malloc(sizeof(runa_value));
            memcpy(args[argc], &value, sizeof(runa_value));
            argc++;
            char *next = runa_token(runa);
            if( next[0] == ')' ) {
                free(next);
                break;
            };

            if( next[0] != ',' ) {
                for( int i = 0; i < argc; i++ ) free(args[i]);
                free(args);
                return runa_send_error(runa, RUNA_INVALID_SYNTAX_IN_CALL, token);
            }

            free(next);
        }

        if( function.arguments > argc )
        /* -> */ return runa_send_error(runa, RUNA_ARGUMENTS_COUNT_WRONG, token);

        runa_stack_push(runa->arguments, args);
        function.function(runa);
        last_did |= FUNCTION_CALLED;
        runa_stack_pop(runa->arguments);

        int n = position;
        for( int i = 0; i < argc; i++ ) {
            if( args[i]->kind == runa_string && (o - n) >= 2 ) free(args[i]->value.string);
            free(args[i]);
        }
        free(args);

        return true;
    }
    else
    if( strcmp(next, "=") == 0 ) {
        free(next);

        char *first = runa_token(runa);
        runa_value value = { .kind = runa_nil, .value.nil = NULL };
        if(! expression(runa, first, &value) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_IN_CALL, token);
        runa_assign_local(runa, token, &value);
    }

    runa_back(runa, next);
    return false;
}

bool local(Runa *runa, char *token) {
    if( strcmp(token, "local") != 0 ) return false;

    int i = 0, v = 0;
    char **identifiers = NULL;
    runa_value **values = NULL;

    /* Iterate identifiers */
    while(1) {
        char *identifier = runa_token(runa);
        if(! isidentifier(identifier) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_IN_LOCAL, identifier);

        identifiers = realloc(identifiers, sizeof(char*) * (i + 1));
        identifiers[i++] = strdup(identifier);

        char *operator = runa_token(runa);

        if(! (operator[0] == ',' || strcmp(operator, "=") == 0) ) {
            runa_back(runa, operator);
            free(identifier);

            /* assign nil to identifiers */
            for( int j = 0; j < i; j++ ) {
                runa_value value = { .kind = runa_nil, .value.nil = NULL };
                runa_push_local(runa, identifiers[j], &value);
            }

            return true;
        }

        free(identifier);
        if( strcmp(operator, "=") == 0 ) {
            free(operator);
            break;
        }
    }

    /* Iterate values */
    int j = 0;
    while(1) {
        char *data = runa_token(runa);
        runa_value value = { .kind = runa_nil, .value.nil = NULL };

        if(! expression(runa, data, &value) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, token);
        free(data);
        values = realloc(values, sizeof(runa_value*) * (v + 1));
        values[v] = malloc(sizeof(runa_value));
        memcpy(values[v], &value, sizeof(runa_value));
        v++;

        runa_assign_local(runa, identifiers[j++], &value);

        char *operator = runa_token(runa);
        if( operator[0] != ',' ) {
            runa_back(runa, operator);

            for( int j = 0; j < i; j++ ) free(identifiers[j++]);
            free(identifiers);

            for( int i = 0; i < v; i++ ) free(values[i]);
            free(values);
            return true;
        }
    }

    return false;
}

void runa_parse(Runa *runa) {
    runa->pushed = NULL;
    while(1) {
        if( runa->error ) break;
        char *token = runa_token(runa);
        bool is_eof = strcmp(RUNA_EOF, token) == 0;
        if( is_eof ) goto dump_token;
        if( statements(runa, token) ) goto dump_token;
        if( local(runa, token) ) goto dump_token;
        if( identifier(runa, token) ) goto dump_token;

        dump_token: {
            free(token);
            if(! is_eof ) continue;
            break;
        };
    }
}

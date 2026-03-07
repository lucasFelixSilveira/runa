#include "runa.h"
#include "lexer.h"
#include "parser.h"
#include "stack.h"
#include "checkout.h"
#include "expressions.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

bool expression(Runa *runa, char *token, runa_value *value) {
    bool already_peeked = false;
    runa_value *peeked_value = NULL;

    if( isidentifier(token) ) {
        char *operator = runa_token(runa);

        if( operator[0] == '[' ) {
            runa_value *table = (runa_value*)malloc(sizeof(runa_value));
            table[0] =  (runa_value) { .kind = runa_nil, .value.nil = NULL };
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

            if( data->kind == runa_nil ) return runa_send_fatal_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, str);

            char *end = runa_token(runa);
            if( end[0] != ']' ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, end);


            free(operator);
            free(access);
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

            runa_value *data = (runa_value*)malloc(sizeof(runa_value));
            data[0] = (runa_value) { .kind = runa_nil, .value.nil = NULL };
            if( isconcat && rhs.kind == runa_string ) {
                data->kind = runa_string;
                char *lhs = runa_value_to_string(peeked_value);
                int size = strlen(lhs) + strlen(rhs.value.string) + 1;
                data->value.string = (char*)malloc(size);
                sprintf(data->value.string, "%s%s", lhs, rhs.value.string);
                memcpy(value, data, sizeof(runa_value));
                return true;
            }
            else return runa_send_error(runa, RUNA_TABLES_CANT_DO_NOTHING_EXCEPT_CONCATENATE, token);
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
            if(! expression(runa, arg, &value) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_IN_CALL, token);
            args = realloc(args, sizeof(runa_value*) * (argc + 1));
            args[argc] = malloc(sizeof(runa_value));
            memcpy(args[argc], &value, sizeof(runa_value));
            argc++;
            char *next = runa_token(runa);
            if( next[0] == ')' ) {
                free(next);
                break;
            };

            if( next[0] != ',' ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_IN_CALL, token);
            free(next);
        }

        if( function.arguments > argc )
        /* -> */ return runa_send_error(runa, RUNA_ARGUMENTS_COUNT_WRONG, token);

        runa_stack_push(runa->arguments, args);
        function.function(runa);
    }

    return false;
}

bool local(Runa *runa, char *token) {
    if( strcmp(token, "local") != 0 ) return false;

    int i = 0, v = 0;
    char **identifiers = (char**)malloc(0);
    runa_value **values = (runa_value**)malloc(0);

    /* Iterate for identifiers */
    while(1) {
        char *identifier = runa_token(runa);
        if(! isidentifier(identifier) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_IN_LOCAL, identifier);
        identifiers = realloc(identifiers, sizeof(char*) * (i + 1));
        identifiers[i++] = identifier;

        char *operator = runa_token(runa);
        if(! (operator[0] == ',' || strcmp(operator, "=") == 0) ) {
            /* Allocate all identifiers as nil */
            runa_back(runa, operator);

            for( int j = 0; j < i; j++ ) {
                runa_value value = (runa_value) { .kind = runa_nil, .value.nil = NULL };
                runa_push_local(runa, identifiers[j], &value);
            }

            return true;
        }

        if( strcmp(operator, "=") == 0 ) break;
    }

    /* Iterate for values */
    int j = 0;
    while(1) {
        char *data = runa_token(runa);
        runa_value *value = malloc(sizeof(runa_value));
        value[0] = (runa_value) { .kind = runa_nil, .value.nil = NULL };
        if(! expression(runa, data, value) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_IN_CALL, token);
        values = realloc(values, sizeof(runa_value*) * (v + 1));
        values[v] = malloc(sizeof(runa_value));
        memcpy(values[v], value, sizeof(runa_value));
        v++;
        runa_assign_local(runa, identifiers[j++], value);

        char *operator = runa_token(runa);
        if( operator[0] != ',' ) {
            runa_back(runa, operator);
            return true;
        }
    }

    return false;
}

void runa_parse(Runa *runa) {
    int x = 10;
    while(x > 0) {
        if( runa->error ) break;
        char *token = runa_token(runa);
        if( local(runa, token) ) continue;
        if( identifier(runa, token) ) continue;
        x -= 1;
    }
}

#include "runa.h"
#include "lexer.h"
#include "parser.h"
#include "stack.h"
#include "checkout.h"
#include "expressions.h"
#include <stdbool.h>
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

            *value = *data;
            char *end = runa_token(runa);
            if( end[0] != ']' ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, end);

            return true;
        }

        bool isoperation = false;

        checktoop(concat, "..");
        checktoop(add, "+");
        checktoop(sub, "-");
        checktoop(div, "/");
        checktoop(mul, "*");
        checktoop(mod, "%");
        checktoop(idiv, "//");

        runa_value *peeked_value = NULL;
        if(! runa_peek_local(runa, token, &peeked_value) ) return runa_send_error(runa, RUNA_UNKNOWN_SYMBOL, token);

        runa_back(runa, operator);

        if( isoperation ) {
            if( peeked_value->kind == runa_string ) return string_expression(runa, token, value);
            if( peeked_value->kind == runa_float || peeked_value->kind == runa_integer ) return numeric_expression(runa, token, value);
        }

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
        runa_value value = { .kind = runa_nil, .value.nil = NULL };
        if(! expression(runa, data, &value) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_IN_CALL, token);
        values = realloc(values, sizeof(runa_value*) * (v + 1));
        values[v] = malloc(sizeof(runa_value));
        memcpy(values[v], &value, sizeof(runa_value));
        v++;
        runa_assign_local(runa, identifiers[j++], &value);

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

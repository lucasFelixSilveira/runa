#include "runa.h"
#include "lexer.h"
#include "stack.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool isidentifier(char *buff) {
    if( buff == NULL ) return false;
    if(! isalpha((unsigned char)buff[0]) ) return false;

    int i = 1;
    while(buff[i] != '\0') {
        if(! isalnum((unsigned char)buff[i]) ) return false;
        i++;
    }

    return true;
}

bool isstring(char *buff) {
    if( buff == NULL ) return false;
    if(! (buff[0] == '"' || buff[0] == '\'') ) return false;
    int last = strlen(buff) - 1;
    if(! (buff[last] == '"' || buff[last] == '\'') ) return false;
    return true;
}

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
        if( operator[0] == ',' || operator[0] == ')' ) {
            runa_back(runa, operator);
            return true;
        }

        // if( strcmp("..") ) {}
    }

    if( isstring(token) ) {
        value->kind = runa_string;
        char *operator = runa_token(runa);
        if( operator[0] == ',' || operator[0] == ')' ) {
            value->value.string = (char*)malloc(strlen(token)-1);
            memcpy(value->value.string, token + 1, strlen(token) - 2);
            value->value.string[strlen(token) - 2] = '\0';
            runa_back(runa, operator);
            return true;
        }
    }

    return false;
}

bool identifier(Runa *runa, char *token) {
    if(! isidentifier(token) ) return false;

    /* Check for all things who
     * starts with a identifier */

    char *next = runa_token(runa);
    if( next[0] == '(' ) {
        runa_function function;
        if(! check_if_is_function(runa, &function, token) )
        /* -> */ return runa_send_error(runa, RUNA_IS_NOT_A_FUNCTION, token);

        runa_value *args = (runa_value*)malloc(0);
        int argc = 0;
        while(1) {
            char *arg = runa_token(runa);
            if( arg[0] == ')' && argc == 0 ) {
                free(arg);
                break;
            };

            runa_value value;
            if(! expression(runa, arg, &value) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_IN_CALL, token);
            args = (runa_value*)realloc(args, sizeof(runa_value) * (argc + 1));
            args[argc++] = value;

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


void runa_parse(Runa *runa) {
    int x = 10;
    while(x > 0) {
        if( runa->error ) break;
        char *token = runa_token(runa);
        if( identifier(runa, token) ) continue;
        x -= 1;
    }
}

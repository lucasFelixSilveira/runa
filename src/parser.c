#include "runa.h"
#include "lexer.h"
#include <ctype.h>
#include <stdbool.h>
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

bool identifier(Runa *runa, char *token) {
    if(! isalpha(token[0]) ) return false;
    /* Check for all things who
     * starts with a identifier */

    char *next = runa_token(runa);
    if( next[0] == '(' ) {
        runa_function function;
        if(! check_if_is_function(runa, &function, token) )
        /* -> */ return runa_send_error(runa, RUNA_IS_NOT_A_FUNCTION, token);

        int argc = 0;
        while(1) {
            char *arg = runa_token(runa);
            if( arg[0] == ')' ) break;
        }

        if( function.arguments < argc )
        /* -> */ return runa_send_error(runa, RUNA_ARGUMENTS_COUNT_WRONG, token);

        function.function(runa);
    }

    return false;
}


void runa_parse(Runa *runa) {
    while(1) {
        if( runa->error ) break;
        char *token = runa_token(runa);
        if( identifier(runa, token) ) continue;
    }
}

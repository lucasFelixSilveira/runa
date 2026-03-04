#include "runa.h"
#include "lexer.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

bool check_if_is_function(Runa *runa, runa_callback *callback, char *identifier) {
    for( int i = 0; i < runa->functions_length; i++ ) {
        runa_functions function = runa->functions[i];
        if( strcmp(function.identifier, identifier) == 0 ) {
            *callback = function.function;
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
        runa_callback callback;
        if(! check_if_is_function(runa, &callback, token) )
        /* -> */ return runa_send_error(runa, RUNA_IS_NOT_A_FUNCTION, token);
    }
}


void runa_parse(Runa *runa) {
    while(1) {
        if( runa->error ) break;
        char *token = runa_token(runa);
        if( identifier(runa, token) ) continue;
    }
}

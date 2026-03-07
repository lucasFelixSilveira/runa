#include "runa.h"
#include "lexer.h"
#include "parser.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

bool string_expression(Runa *runa, char *token, runa_value *value) {
    value->kind = runa_string;
    char *operator = runa_token(runa);
    int length = strlen(token) - 2;

    value->value.string = (char*)malloc(strlen(token) - 1);
    memcpy(value->value.string, token + 1, length);
    value->value.string[length] = '\0';

    while(1) {
        if( strcmp(operator, "..") != 0 ) {
            runa_back(runa, operator);
            return true;
        }

        char *data = runa_token(runa);
        runa_value rhs = { .kind = runa_nil, .value.nil = NULL };
        if(! expression(runa, data, &rhs) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, token);

        char *text = runa_value_to_string(&rhs);
        value->value.string = realloc(value->value.string, length + strlen(text) - 1);
        sprintf(value->value.string, "%s%s", value->value.string, text);
        length = strlen(text);
        free(text);
        free(operator);
        operator = runa_token(runa);
        continue;

    }
}

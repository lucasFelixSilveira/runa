#include "runa.h"
#include "lexer.h"
#include "parser.h"
#include "checkout.h"
#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool table_expression(Runa *runa, char *token, runa_value *value) {
    value->kind = runa_table;
    runa_vector *table = runa_vector_new(128);

    while(1) {
        char *identifier = runa_token(runa);
        if( strcmp(identifier, "}") == 0 ) break;

        char *next = runa_token(runa);

        if( isidentifier(identifier) && strcmp(next, "=") == 0 ) {
            char *data;
            if( strcmp(next, "=") == 0 ) {
                data = runa_token(runa);
            }
            else if( strcmp(next, ",") == 0 ) {
                runa_back(runa, next);
                data = identifier;
            }
            else return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, token);

            runa_value rhs = { .kind = runa_nil, .value.nil = NULL };
            if(! expression(runa, data, &rhs) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, token);
            free(data);

            runa_value *rhs_value = (runa_value*)malloc(sizeof(runa_value));
            *rhs_value = rhs;
            runa_table_field *field = (runa_table_field*)malloc(sizeof(runa_table_field*));
            *field = (runa_table_field) {
                .identifier = identifier,
                .value = (void*)rhs_value
            };

            runa_vector_append(table, (void*)field);
        }
        else return runa_send_fatal_error(runa, RUNA_TABLES_CANT_DO_NOTHING_EXCEPT_CONCATENATE, token);

        char *after = runa_token(runa);
        if( strcmp(after, ",") == 0 ) continue;
        if( strcmp(after, "}") == 0 ) break;
        return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, token);
    }

    value->value.table = (void*)table;
    return true;
}

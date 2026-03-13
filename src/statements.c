#include "lexer.h"
#include "parser.h"
#include "runa.h"
#include <iso646.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool if_statement(Runa *runa);

bool statements(Runa *runa, char *token) {
    if( strcmp(token, "if") == 0 ) return if_statement(runa);
    return false;
}

const char __not = 1 << 0;
const char __and = 1 << 1;
const char __or  = 1 << 2;
char mod = 0;
bool state = false;

void push_state(bool value) {
    if( mod & __and ) state = state && ((mod & __not) ? !value : value);
    else if( mod & __or ) state = state || ((mod & __not) ? !value : value);
    else state = (mod & __not) ? !value : value;
    mod &= ~(__not | __and | __or);
}

bool is_equal(runa_value *v_lhs, runa_value *v_rhs) {
    if( v_lhs->kind != v_rhs->kind ) return false;
    int kind = v_lhs->kind;
    if( kind == runa_string ) return strcmp(v_lhs->value.string, v_rhs->value.string) == 0;
    if( kind == runa_integer ) return v_lhs->value.integer == v_rhs->value.integer;
    if( kind == runa_float ) return v_lhs->value._float == v_rhs->value._float;
    return false;
}


bool is_greater(runa_value *v_lhs, runa_value *v_rhs) {
    if( v_lhs->kind != v_rhs->kind ) return false;
    int kind = v_lhs->kind;
    if( kind == runa_string ) return strcmp(v_lhs->value.string, v_rhs->value.string) > 0;
    if( kind == runa_integer ) return v_lhs->value.integer > v_rhs->value.integer;
    if( kind == runa_float ) return v_lhs->value._float > v_rhs->value._float;
    return false;
}


bool is_less(runa_value *v_lhs, runa_value *v_rhs) {
    if( v_lhs->kind != v_rhs->kind ) return false;
    int kind = v_lhs->kind;
    if( kind == runa_string ) return strcmp(v_lhs->value.string, v_rhs->value.string) < 0;
    if( kind == runa_integer ) return v_lhs->value.integer < v_rhs->value.integer;
    if( kind == runa_float ) return v_lhs->value._float < v_rhs->value._float;
    return false;
}


bool if_statement(Runa *runa) {
    while(1) {
        char *lhs = runa_token(runa);
        if( strcmp(lhs, "not") == 0 ) {
            free(lhs);
            lhs = runa_token(runa);
            mod |= __not;
        }

        runa_value v_lhs = {0};
        if(! expression(runa, lhs, &v_lhs) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, lhs);
        free(lhs);

        char *operator = runa_token(runa);
        if( strcmp(operator, "then") == 0 ) {
            free(operator);
            if( v_lhs.kind == runa_nil     ) push_state(false);
            if( v_lhs.kind == runa_string  ) push_state(strlen(v_lhs.value.string) != 0);
            if( v_lhs.kind == runa_integer ) push_state(v_lhs.value.integer != 0);
            if( v_lhs.kind == runa_float   ) push_state(v_lhs.value._float != 0.0);
            break;
        }

        if( strcmp(operator, "and") == 0 ) {
            mod |= __and;
            free(operator);
            continue;
        }

        if( strcmp(operator, "or") == 0 ) {
            mod |= __or;
            free(operator);
            continue;
        }

        char *rhs = runa_token(runa);
        runa_value v_rhs = {0};
        if(! expression(runa, lhs, &v_rhs) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, rhs);
        free(rhs);

        if( strcmp(operator, "==") == 0 ) push_state(is_equal(&v_lhs, &v_rhs));
        if( strcmp(operator, "~=") == 0 ) push_state(!is_equal(&v_lhs, &v_rhs));
        if( strcmp(operator, ">=") == 0 ) push_state(is_equal(&v_lhs, &v_rhs) || is_greater(&v_lhs, &v_rhs));
        if( strcmp(operator, "<=") == 0 ) push_state(is_equal(&v_lhs, &v_rhs) || is_less(&v_lhs, &v_rhs));
        if( strcmp(operator, ">")  == 0 ) push_state(is_greater(&v_lhs, &v_rhs));
        if( strcmp(operator, "<")  == 0 ) push_state(is_less(&v_lhs, &v_rhs));

        char *last = runa_token(runa);

        if( strcmp(last, "and") == 0 ) mod |= __and;
        if( strcmp(last, "or")  == 0 ) mod |= __or;
        if( strcmp(last, "then") == 0 ) {
            free(last);
            break;
        }

        free(operator);
    }

    if(! state ) {
        int i = 0, j = 1;
        char *data;
        for(; j > 0; i++ ) {
            data = runa_token(runa);
            if( strlen(data) == 0 ) break;
            j += ( strcmp("end", data)  == 0 ) ? -1
               : ( strcmp("then", data) == 0 || strcmp("do", data) == 0 ) ? 1 : 0;
            free(data);
        }
        if( j != 0 ) runa_send_error(runa, RUNA_UNMATCHED_END, NULL);
    }

    return true;
}

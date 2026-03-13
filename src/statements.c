#include "lexer.h"
#include "parser.h"
#include "runa.h"
#include "stack.h"
#include <iso646.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


bool if_statement(Runa *runa);
bool else_statement(Runa *runa);
bool elseif_statement(Runa *runa);
bool end_statement(Runa *runa);

bool statements(Runa *runa, char *token) {
    if( strcmp(token, "if") == 0 ) return if_statement(runa);
    if( strcmp(token, "else") == 0 ) return else_statement(runa);
    if( strcmp(token, "elseif") == 0 ) return elseif_statement(runa);
    if( strcmp(token, "end") == 0 ) return end_statement(runa);
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

bool else_statement(Runa *runa) {
    bool last = runa_stack_peek(runa->if_stack);

    if( last ) {
        int depth = 1;
        while(depth > 0) {
            char *tok = runa_token(runa);
            if(! tok ) return runa_send_error(runa,RUNA_UNMATCHED_END,NULL);

            if( strcmp(tok, "if") == 0 ) depth++;
            else if( strcmp(tok, "end") == 0 ) depth--;

            free(tok);
        }
    }

    return true;
}

bool elseif_statement(Runa *runa) {
    bool last = runa_stack_peek(runa->if_stack);

    if( last ) {
        int depth = 1;
        while(depth > 0) {
            char *tok = runa_token(runa);
            if(! tok ) return runa_send_error(runa,RUNA_UNMATCHED_END,NULL);

            if( strcmp(tok, "if") == 0 ) depth++;
            else if( strcmp(tok, "end") == 0 ) depth--;
            else if( strcmp(tok, "elseif") == 0 && depth == 1 ) break;
            else if( strcmp(tok, "else") == 0 && depth == 1 ) break;

            free(tok);
        }

        return true;
    }

    return if_statement(runa);
}

bool end_statement(Runa *runa) {
    bool *b = runa_stack_pop(runa->if_stack);
    free(b);
    return true;
}

bool if_statement(Runa *runa) {
    state = false;
    mod = 0;
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
        int depth = 1;
        while(depth > 0) {
            char *tok = runa_token(runa);
            if( strcmp(tok, "if") == 0 ) depth++;
            else if( strcmp(tok, "end") == 0 ) depth--;
            else if( strcmp(tok, "else") == 0 && depth == 1 ) {
                runa_back(runa, tok);
                break;
            }
            free(tok);
        }
    }

    bool *b = (bool*)malloc(1);
    *b = state;
    runa_stack_push(runa->if_stack, b);
    return true;
}

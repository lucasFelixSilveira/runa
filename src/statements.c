#include "compression.h"
#include "checkout.h"
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
bool function_statement(Runa *runa);
bool return_statement(Runa *runa);

bool statements(Runa *runa, char *token) {
    if( strcmp(token, "if") == 0 ) return if_statement(runa);
    if( strcmp(token, "else") == 0 ) return else_statement(runa);
    if( strcmp(token, "elseif") == 0 ) return elseif_statement(runa);
    if( strcmp(token, "end") == 0 ) return end_statement(runa);
    if( strcmp(token, "function") == 0 ) return function_statement(runa);
    if( strcmp(token, "return") == 0 ) return return_statement(runa);
    return false;
}

const char __not = 1 << 0;
const char __and = 1 << 1;
const char __or  = 1 << 2;


void push_state(Runa *runa, bool value) {
    if( runa->mod & __and ) runa->state = runa->state && ((runa->mod & __not) ? !value : value);
    else if( runa->mod & __or ) runa->state = runa->state || ((runa->mod & __not) ? !value : value);
    else runa->state = (runa->mod & __not) ? !value : value;
    runa->mod &= ~(__not | __and | __or);
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
    runa->state = false;
    runa->mod = 0;
    while(1) {
        char *lhs = runa_token(runa);
        if( strcmp(lhs, "not") == 0 ) {
            free(lhs);
            lhs = runa_token(runa);
            runa->mod |= __not;
        }

        runa_value v_lhs = {0};
        if(! expression(runa, lhs, &v_lhs) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, lhs);
        free(lhs);

        char *operator = runa_token(runa);
        if( strcmp(operator, "then") == 0 ) {
            free(operator);
            if( v_lhs.kind == runa_nil     ) push_state(runa, false);
            if( v_lhs.kind == runa_string  ) push_state(runa, strlen(v_lhs.value.string) != 0);
            if( v_lhs.kind == runa_integer ) push_state(runa, v_lhs.value.integer != 0);
            if( v_lhs.kind == runa_float   ) push_state(runa, v_lhs.value._float != 0.0);
            break;
        }

        if( strcmp(operator, "and") == 0 ) {
            runa->mod |= __and;
            free(operator);
            continue;
        }

        if( strcmp(operator, "or") == 0 ) {
            runa->mod |= __or;
            free(operator);
            continue;
        }

        char *rhs = runa_token(runa);
        runa_value v_rhs = {0};
        if(! expression(runa, rhs, &v_rhs) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, rhs);
        free(rhs);

        if( strcmp(operator, "==") == 0 ) push_state(runa, is_equal(&v_lhs, &v_rhs));
        if( strcmp(operator, "~=") == 0 ) push_state(runa, !is_equal(&v_lhs, &v_rhs));
        if( strcmp(operator, ">=") == 0 ) push_state(runa, is_equal(&v_lhs, &v_rhs) || is_greater(&v_lhs, &v_rhs));
        if( strcmp(operator, "<=") == 0 ) push_state(runa, is_equal(&v_lhs, &v_rhs) || is_less(&v_lhs, &v_rhs));
        if( strcmp(operator, ">")  == 0 ) push_state(runa, is_greater(&v_lhs, &v_rhs));
        if( strcmp(operator, "<")  == 0 ) push_state(runa, is_less(&v_lhs, &v_rhs));

        char *last = runa_token(runa);

        if( strcmp(last, "and") == 0 ) runa->mod |= __and;
        if( strcmp(last, "or")  == 0 ) runa->mod |= __or;
        if( strcmp(last, "then") == 0 ) {
            free(last);
            break;
        }

        free(operator);
    }

    if(! runa->state ) {
        int depth = 1;
        while(depth > 0) {
            char *tok = runa_token(runa);
            if( strcmp(tok, "if") == 0 ) depth++;
            else if( strcmp(tok, "end") == 0 ) depth--;
            else if( strcmp(tok, "else") == 0 && depth == 1 ) break;
            free(tok);
        }
    }

    bool *b = (bool*)malloc(1);
    *b = runa->state;
    runa_stack_push(runa->if_stack, b);
    return true;
}

bool function_statement(Runa *runa) {
    char *name = runa_token(runa);
    if( (! name ) || (! isidentifier(name)) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX, name);

    char *open = runa_token(runa);
    if( (! open) || open[0] != '(' ) {
        free(name);
        return runa_send_error(runa, RUNA_INVALID_SYNTAX, open);
    }
    free(open);

    char **params = NULL;
    int argc = 0;

    while(1) {
        char *tok = runa_token(runa);
        if( tok[0] == ')' ) {
            free(tok);
            break;
        }

        if(! isidentifier(tok) ) {
            free(tok);
            free(name);
            for(int i = 0; i < argc; i++) free(params[i]);
            free(params);
            return runa_send_error(runa, RUNA_INVALID_SYNTAX, tok);
        }

        char *param = strdup(tok);
        free(tok);

        char **tmp = realloc(params, sizeof(char*) * (argc + 1));
        if(! tmp ) {
            free(param);
            free(name);
            for(int i = 0; i < argc; i++) free(params[i]);
            free(params);
            return runa_send_error(runa, RUNA_OUT_OF_MEMORY, NULL);
        }

        params = tmp;
        params[argc++] = param;

        char *next = runa_token(runa);

        if( next[0] == ')' ) {
            free(next);
            break;
        }

        if( next[0] != ',' ) {
            free(next);
            free(name);
            for( int i = 0; i < argc; i++ ) free(params[i]);
            free(params);
            return runa_send_error(runa, RUNA_INVALID_SYNTAX, next);
        }

        free(next);
    }

    int depth = 1;

    char *buffer = NULL;
    size_t size = 0;

    while(depth > 0) {
        char *tok = runa_token(runa);

        if(! tok ) {
            free(name);
            for( int i = 0; i < argc; i++ ) free(params[i]);
            free(params);
            free(buffer);
            return runa_send_error(runa, RUNA_UNMATCHED_END, NULL);
        }

        if( strcmp(tok, "function") == 0 ||
            strcmp(tok, "while") == 0 ||
            strcmp(tok, "if") == 0 ) {
            depth++;
        }
        else if( strcmp(tok, "end") == 0 ) {
            depth--;
        }

        if( depth > 0 ) {
            size_t len = strlen(tok);

            char *tmp = realloc(buffer, size + len + 2);
            if(!tmp) {
                free(tok);
                free(name);
                for(int i = 0; i < argc; i++) free(params[i]);
                free(params);
                free(buffer);
                return runa_send_error(runa, RUNA_OUT_OF_MEMORY, NULL);
            }

            buffer = tmp;
            memcpy(buffer + size, tok, len);
            size += len;
            buffer[size++] = '\n';
        }

        free(tok);
    }

    char *tmp = realloc(buffer, size + 1);
    if(! tmp ) {
        free(name);
        for( int i = 0; i < argc; i++ ) free(params[i]);
        free(params);
        free(buffer);
        return runa_send_error(runa, RUNA_OUT_OF_MEMORY, NULL);
    }
    buffer = tmp;
    buffer[size] = '\0';

    char *compressed = NULL;
    int compressed_size = 0;

    if(! runa_compress(buffer, size, &compressed, &compressed_size) ) {
        free(buffer);
        free(name);
        for(int i = 0; i < argc; i++) free(params[i]);
        free(params);
        return runa_send_error(runa, RUNA_COMPRESSION_FAILED, NULL);
    }

    free(buffer);

    runa_function *tmpf = realloc(
        runa->functions,
        sizeof(runa_function) * (runa->functions_length + 1)
    );

    if(!tmpf) {
        free(compressed);
        free(name);
        for(int i = 0; i < argc; i++) free(params[i]);
        free(params);
        return runa_send_error(runa, RUNA_OUT_OF_MEMORY, NULL);
    }

    runa->functions = tmpf;

    runa_function *f = &runa->functions[runa->functions_length++];

    f->identifier = strdup(name);
    f->params = params;
    f->arguments = argc;
    f->function = NULL;

    f->body = compressed;
    f->body_size = compressed_size;

    free(name);
    return true;
}

bool return_statement(Runa *runa) {
    char *tok = runa_token(runa);

    if( tok == NULL || tok[0] == '\0' || strcmp(tok, "end") == 0 ) {
        runa->result = NULL;
        runa->should_leave = true;
        if (tok) free(tok);
        return true;
    }

    runa_value value = {0};
    if(! expression(runa, tok, &value) ) {
        free(tok);
        return false;
    }

    runa_value *heap = malloc(sizeof(runa_value));
    memcpy(heap, &value, sizeof(runa_value));

    if( value.kind == runa_string && value.value.string != NULL ) {
        heap->value.string = strdup(value.value.string);
        if (isstring(tok)) free(value.value.string);
    }

    runa->result = heap;
    runa->should_leave = true;

    free(tok);
    return true;
}

#include "runa.h"
#include "lexer.h"
#include "checkout.h"
#include "parser.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

bool is_operator(char *s) {
    return !strcmp(s, "+") ||
           !strcmp(s, "-") ||
           !strcmp(s, "*") ||
           !strcmp(s, "/") ||
           !strcmp(s, "%") ||
           !strcmp(s, "//");
}

int operator_precedence(char *op) {
    if(! strcmp(op, "+") || !strcmp(op, "-") ) return 1;
    if(! strcmp(op, "*") || !strcmp(op, "/") || !strcmp(op, "%") || !strcmp(op, "//") ) return 2;
    return 0;
}

bool resolve_numeric_token(Runa *runa, char *token, char **out) {
    if( isnumber(token) ) {
        *out = strdup(token);
        return true;
    }

    if( isidentifier(token) ) {
        runa_value *v = NULL;
        if(! runa_peek_local(runa, token, &v) ) return false;

        *out = runa_value_to_string(v);
        return true;
    }

    return false;
}

bool numeric_expression(Runa *runa, char *token, runa_value *value) {
    char *operator = runa_token(runa);
    bool isoperation = false;

    checktoop(add, "+");
    checktoop(sub, "-");
    checktoop(div, "/");
    checktoop(mul, "*");
    checktoop(mod, "%");
    checktoop(idiv, "//");

    char *first;

    if(! resolve_numeric_token(runa, token, &first) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, token);

    if(! isoperation ) {

        runa_back(runa, operator);

        long double result = strtold(first, NULL);

        if( (long long)result == result ) {
            value->kind = runa_integer;
            value->value.integer = (long long)result;
        } else {
            value->kind = runa_float;
            value->value._float = result;
        }

        free(first);
        return true;
    }

    int len = 0;
    int cap = 8;

    char **tokens = malloc(sizeof(char*) * cap);

    tokens[len++] = first;
    tokens[len++] = operator;

    while(1) {
        char *data = runa_token(runa);
        char *num;

        if(! resolve_numeric_token(runa, data, &num) ) return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, data);

        if( len >= cap ) {
            cap *= 2;
            tokens = realloc(tokens, sizeof(char*) * cap);
        }

        tokens[len++] = num;

        operator = runa_token(runa);
        isoperation = false;

        checktoop(add, "+");
        checktoop(sub, "-");
        checktoop(div, "/");
        checktoop(mul, "*");
        checktoop(mod, "%");
        checktoop(idiv, "//");

        if(! isoperation ) {
            runa_back(runa, operator);
            break;
        }

        if( len >= cap ) {
            cap *= 2;
            tokens = realloc(tokens, sizeof(char*) * cap);
        }

        tokens[len++] = operator;
    }

    char **output = malloc(sizeof(char*) * len);
    char **stack  = malloc(sizeof(char*) * len);

    int oi = 0;
    int si = 0;

    for( int i = 0; i < len; i++ ) {
        char *t = tokens[i];
        if(! is_operator(t) ) output[oi++] = t;
        else {
            while(si > 0 && operator_precedence(stack[si-1]) >= operator_precedence(t)) output[oi++] = stack[--si];
            stack[si++] = t;
        }
    }

    while(si > 0) output[oi++] = stack[--si];

    long double *numstack = malloc(sizeof(long double) * oi);
    int ni = 0;

    for( int i = 0; i < oi; i++ ) {
        char *t = output[i];

        if(! is_operator(t) ) {
            numstack[ni++] = strtold(t, NULL);
            continue;
        }

        long double b = numstack[--ni];
        long double a = numstack[--ni];

        if(! strcmp(t, "+") ) numstack[ni++] = a + b;
        else if(! strcmp(t, "-") ) numstack[ni++] = a - b;
        else if(! strcmp(t, "*") ) numstack[ni++] = a * b;
        else if(! strcmp(t, "/") ) numstack[ni++] = a / b;
        else if(! strcmp(t, "%") ) numstack[ni++] = (long long)a % (long long)b;
        else if(! strcmp(t, "//") ) numstack[ni++] = (long long)(a / b);
    }

    long double result = numstack[0];

    if( (long long)result == result ) {
        value->kind = runa_integer;
        value->value.integer = (long long)result;
    } else {
        value->kind = runa_float;
        value->value._float = result;
    }

    for( int i = 0; i < len; i++ ) free(tokens[i]);
    free(tokens);

    return true;
}

#include "runa.h"
#include "lexer.h"
#include "stack.h"
#include "checkout.h"
#include <math.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define checktoop(identifier, literal) \
    bool is##identifier = strcmp(operator, literal) == 0; \
    isoperation = isoperation || is##identifier

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

bool expression(Runa *runa, char *token, runa_value *value);

bool is_operator(const char *s) {
    return !strcmp(s,"+") ||
           !strcmp(s,"-") ||
           !strcmp(s,"*") ||
           !strcmp(s,"/") ||
           !strcmp(s,"%") ||
           !strcmp(s,"//");
}

int operator_precedence(const char *op) {
    if(!strcmp(op,"+") || !strcmp(op,"-")) return 1;
    if(!strcmp(op,"*") || !strcmp(op,"/") || !strcmp(op,"%") || !strcmp(op,"//")) return 2;
    return 0;
}

bool resolve_numeric_token(Runa *runa, char *token, char **out) {

    if(isnumber(token)) {
        *out = strdup(token);
        return true;
    }

    if(isidentifier(token)) {

        runa_value *v = NULL;

        if(!runa_peek_local(runa, token, &v))
            return false;

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

    if(!resolve_numeric_token(runa, token, &first))
        return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, token);

    if(!isoperation) {

        runa_back(runa, operator);

        long double result = strtold(first, NULL);

        if((long long)result == result) {
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

        if(!resolve_numeric_token(runa, data, &num))
            return runa_send_error(runa, RUNA_INVALID_SYNTAX_OF_EXPRESSION, data);

        if(len >= cap) {
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

        if(!isoperation) {
            runa_back(runa, operator);
            break;
        }

        if(len >= cap) {
            cap *= 2;
            tokens = realloc(tokens, sizeof(char*) * cap);
        }

        tokens[len++] = operator;
    }

    /* SHUNTING YARD */

    char **output = malloc(sizeof(char*) * len);
    char **stack  = malloc(sizeof(char*) * len);

    int oi = 0;
    int si = 0;

    for(int i = 0; i < len; i++) {

        char *t = tokens[i];

        if(!is_operator(t)) {
            output[oi++] = t;
        } else {

            while(si > 0 &&
                  operator_precedence(stack[si-1]) >= operator_precedence(t)) {

                output[oi++] = stack[--si];
            }

            stack[si++] = t;
        }
    }

    while(si > 0)
        output[oi++] = stack[--si];

    /* EVALUATE */

    long double *numstack = malloc(sizeof(long double) * oi);
    int ni = 0;

    for(int i = 0; i < oi; i++) {

        char *t = output[i];

        if(!is_operator(t)) {
            numstack[ni++] = strtold(t, NULL);
            continue;
        }

        long double b = numstack[--ni];
        long double a = numstack[--ni];

        if(!strcmp(t,"+")) numstack[ni++] = a + b;
        else if(!strcmp(t,"-")) numstack[ni++] = a - b;
        else if(!strcmp(t,"*")) numstack[ni++] = a * b;
        else if(!strcmp(t,"/")) numstack[ni++] = a / b;
        else if(!strcmp(t,"%")) numstack[ni++] = (long long)a % (long long)b;
        else if(!strcmp(t,"//")) numstack[ni++] = (long long)(a / b);
    }

    long double result = numstack[0];

    if((long long)result == result) {
        value->kind = runa_integer;
        value->value.integer = (long long)result;
    } else {
        value->kind = runa_float;
        value->value._float = result;
    }

    return true;
}

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

bool expression(Runa *runa, char *token, runa_value *value) {
    if( isidentifier(token) ) {
        char *operator = runa_token(runa);
        bool isoperation = false;

        checktoop(concat, "..");
        checktoop(add, "+");
        checktoop(sub, "-");
        checktoop(div, "/");
        checktoop(mul, "*");
        checktoop(mod, "%");
        checktoop(idiv, "//");

        runa_value *peeked_value = NULL;
        if(! runa_peek_local(runa, token, &peeked_value)) {
            return runa_send_error(runa, RUNA_UNKNOWN_SYMBOL, token);
        }

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

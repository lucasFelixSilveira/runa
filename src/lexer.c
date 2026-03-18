#include "runa.h"
#include "lexer.h"
#include "checkout.h"
#include "stack.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long long position = 0;
int codeid = 0;
int pos[64];

void runa_back(Runa *runa, char *token) {
    runa->pushed = token;
}

char *runa_token(Runa *runa) {
    char buffer[2048] = {0};
    position += 1;
    if( runa->pushed ) {
        char *t = runa->pushed;
        runa->pushed = NULL;
        return t;
    }

    if( runa->code_stack->length > 0 ) {
        char *code = runa_stack_peek(runa->code_stack);
        int len = strlen(code);
        int locale = pos[codeid];
        int i = locale;
        int j = 0;

        for(; i < len; i++) {
            if(i >= (len - 2)) {
                char *data = runa_stack_pop(runa->code_stack);
                buffer[j++] = code[i];
                free(data);
                char *code = strdup(buffer);
                pos[codeid--] = 0;
                buffer[0] = 0;
                runa->should_leave = true;
                return code;
            }

            if( code[i] == '\n' ) break;
            buffer[j++] = code[i];
        }

        buffer[j] = '\0';
        pos[codeid] = i + 1;

        return strdup(buffer);
    }

    FILE *file = runa->file;
    int length = 0;

    while(1) {
        int ci = getc(file);
        if( ci == EOF ) return strdup(RUNA_EOF);
        char c = (char)ci;

        if( c == '"' || c == '\'' ) {
            char quote = c;
            buffer[length++] = quote;

            while(1) {
                int xi = getc(file);
                if( xi == EOF ) break;
                char x = (char)xi;

                buffer[length++] = x;
                if( x == quote ) break;

                if( x == '\\' ) {
                    int next = getc(file);
                    if ( next == EOF ) break;
                    buffer[length++] = (char)next;
                }
            }

            char *token = malloc(length + 1);
            if(! token ) return NULL;
            memcpy(token, buffer, length);
            token[length] = '\0';
            return token;
        }

        if( isspace(c) ) {
            if( length > 0 ) {
                char *token = malloc(length + 1);
                if (!token) return NULL;
                memcpy(token, buffer, length);
                token[length] = '\0';
                return token;
            }
            continue;
        }

        if(! isalnum(c) && c != '_' ) {
            if( length > 0 && isinteger(buffer) && c == '.' ) {
                buffer[length++] = c;
                buffer[length] = '\0';
                continue;
            }

            if( length > 0 ) {
                ungetc(c, file);
                char *token = malloc(length + 1);
                if(! token ) return NULL;
                memcpy(token, buffer, length);
                token[length] = '\0';
                return token;
            }

            char buff[3] = {c, 0, 0};
            int size = 1;

            int next = getc(file);
            char x = (char)next;

            if( (c == '=' && x == '=') ||
                (c == '!' && x == '=') ||
                (c == '<' && x == '=') ||
                (c == '>' && x == '=') ||
                (c == '-' && x == '-') ||
                (c == '.' && x == '.') ||
                (c == '&' && x == '&') ||
                (c == '/' && x == '/') ||
                (c == '|' && x == '|') ||
                (c == '~' && x == '='))
            {
                buff[1] = x;
                size = 2;
            }
            else ungetc(x, file);

            if( c == '-' && x == '-' ) {
                while ((next = getc(file)) != EOF && next != '\n');
                continue;
            }

            char *token = malloc(size + 1);
            if(! token ) return NULL;
            memcpy(token, buff, size);
            token[size] = '\0';
            return token;
        }

        buffer[length++] = c;
        buffer[length] = '\0';

        if( length >= (int)sizeof(buffer) - 1 ) {
            break;
        }
    }

    if( length > 0 ) {
        char *token = malloc(length + 1);
        if (!token) return NULL;
        memcpy(token, buffer, length);
        token[length] = '\0';
        return token;
    }

    return strdup("");
}

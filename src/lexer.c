#include "runa.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void runa_back(Runa *runa, char *token) {
    for( int i = strlen(token) - 1; i >= 0; i-- )
    /* -> */ungetc(token[i], runa->file);
    free(token);
}

char *runa_token(Runa *runa) {
    FILE *file = runa->file;
    char buffer[2048] = {0};
    int length = 0;

    while (1) {
        int ci = getc(file);
        if( ci == EOF ) break;
        char c = (char)ci;

        if (c == '"' || c == '\'') {
            char quote = c;
            buffer[length++] = quote;

            while (1) {
                int xi = getc(file);
                if (xi == EOF) break;
                char x = (char)xi;

                buffer[length++] = x;
                if( x == quote ) break;

                if( x == '\\' ) {
                    int next = getc(file);
                    if (next == EOF) break;
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

        if(! isalnum(c) ) {
            if( length > 0 ) {
                ungetc(c, file);
                char *token = malloc(length + 1);
                if (!token) return NULL;
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

    if (length > 0) {
        char *token = malloc(length + 1);
        if (!token) return NULL;
        memcpy(token, buffer, length);
        token[length] = '\0';
        return token;
    }

    return strdup("");
}

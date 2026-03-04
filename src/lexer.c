#include "runa.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *runa_token(Runa *runa) {
    FILE *file = runa->file;
    char buffer[2048] = { 0 };
    int length = 0;

    while(1) {
        char c = getc(file);
        if( c == EOF ) break;

        if( (c == '"' ||  c == '\'') && length > 0 ) {
            ungetc(c, file);
            char *alloc = (char*)malloc(length + 1);
            memcpy(alloc, buffer, length + 1);
            return alloc;
        } else if( c == '"' ||  c == '\'' ) {
            char x;
            while( (x = getc(file)) != c ) buffer[length++] = x;
        };

        if( isspace(c) && length > 0 ) {
            char *alloc = (char*)malloc(length + 1);
            memcpy(alloc, buffer, length + 1);
            return alloc;
        } else if( isspace(c) ) continue;

        if( (! isalnum(c) ) && length > 0 ) {
            ungetc(c, file);
            char *alloc = (char*)malloc(length + 1);
            memcpy(alloc, buffer, length + 1);
            return alloc;
        } else if(! isalnum(c) ) {
            char x = getc(file);
            char buff[3] = { x, 0, 0 };
            int size = 2;

            if(
                   (c == '=' && x == '=')
                || (c == '~' && x == '=')
            ) {
                buff[1] = x;
                size = 3;
            }

            ungetc(x, file);
            char *alloc = (char*)malloc(size);
            memcpy(alloc, buff, size);
        };

        buffer[length++] = c;
        buffer[length] = 0;
    }

    if( length > 0 ) {
        char *alloc = (char*)malloc(length + 1);
        memcpy(alloc, buffer, length + 1);
        return alloc;
    }

    char *alloc = (char*)malloc(1);
    return alloc;
}

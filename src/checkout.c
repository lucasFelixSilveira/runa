#include <ctype.h>
#include <stdbool.h>
#include <string.h>
bool isidentifier(char *buff) {
    if( buff == NULL ) return false;
    if(! isalpha((unsigned char)buff[0]) ) return false;

    int i = 1;
    while(buff[i] != '\0') {
        if(! isalnum((unsigned char)buff[i]) ) return false;
        i++;
    }

    return true;
}

bool isinteger(char *buff) {
    int i = 0;

    while(buff[i] != '\0') {
        if(! isdigit((unsigned char)buff[i]) ) return false;
        i++;
    }

    return true;
}

bool isfloat(char *buff) {
    int i = 0;
    bool dot = false;

    if( buff[0] == '\0' ) return false;

    while(buff[i] != '\0') {

        if( buff[i] == '.' ) {
            if( dot ) return false;
            dot = true;
            i++;
            continue;
        }

        if(! isdigit((unsigned char)buff[i]) ) return false;
        i++;
    }

    return dot;
}

#define isnumber(value) \
    ( isinteger(value) || isfloat(value) )

bool isstring(char *buff) {
    if( buff == NULL ) return false;
    if(! (buff[0] == '"' || buff[0] == '\'') ) return false;
    int last = strlen(buff) - 1;
    if(! (buff[last] == '"' || buff[last] == '\'') ) return false;
    return true;
}

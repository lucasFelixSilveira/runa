#ifndef RUNA_CHECKOUT_H
#define RUNA_CHECKOUT_H

#include <stdbool.h>

#define isnumber(value) \
    ( isinteger(value) || isfloat(value) )

bool isidentifier(char *buff);
bool isinteger(char *buff);
bool isfloat(char *buff);
bool isstring(char *buff);

#endif

#ifndef RUNA_EXPRESSIONS_H
#define RUNA_EXPRESSIONS_H

#include "runa.h"
#include <stdbool.h>

bool string_expression(Runa *runa, char *token, runa_value *value);
bool numeric_expression(Runa *runa, char *token, runa_value *value);
#endif

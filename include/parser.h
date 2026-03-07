#ifndef RUNA_PARSER_H
#define RUNA_PARSER_H

#include "runa.h"

#define checktoop(identifier, literal) \
    bool is##identifier = strcmp(operator, literal) == 0; \
    isoperation = isoperation || is##identifier

void runa_parse(Runa *runa);
bool expression(Runa *runa, char *token, runa_value *value);

#endif

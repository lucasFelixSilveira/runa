#ifndef RUNA_LEXER_H
#define RUNA_LEXER_H

#include "runa.h"
#define RUNA_EOF "__RUNA_EOF"

void runa_back(Runa *runa, char *token);
char *runa_token(Runa *runa);

#endif

#ifndef RUNA_STD_H
#define RUNA_STD_H

#include "runa.h"

#define ___runa_use_std__(runa) \
    ( runa_push_function(runa, "print", (runa_callback)runa_print, 1) )

void runa_print(Runa *runa);

#endif

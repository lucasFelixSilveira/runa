#ifndef RUNA_PARSER_H
#define RUNA_PARSER_H

#include "runa.h"

#define checktoop(identifier, literal) \
    bool is##identifier = strcmp(operator, literal) == 0; \
    isoperation = isoperation || is##identifier

static runa_stack *call_stack = NULL;

call_state* get_current_state();
void push_call_state(Runa *runa);
void pop_call_state(Runa *runa);
void set_call_result(Runa *runa, runa_value *result);

void init_call_stack(void);
void free_call_stack(void);

void runa_parse(Runa *runa);
bool expression(Runa *runa, char *token, runa_value *value);

#endif

#ifndef RUNA_STACK_H
#define RUNA_STACK_H

#include <stddef.h>

typedef struct {
    size_t length;
    size_t capacity;
    void **values;
} runa_stack;

runa_stack *runa_stack_new(size_t initial_capacity);

void runa_stack_push(runa_stack *stack, void *value);
void *runa_stack_pop(runa_stack *stack);
void *runa_stack_peek(runa_stack *stack);

void runa_stack_reserve(runa_stack *stack, size_t new_capacity);
void runa_stack_free(runa_stack *stack);
void runa_stack_free_all(runa_stack *stack, void (*destructor)(void *));

#endif

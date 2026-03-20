#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

#define RUNA_STACK_MIN_CAPACITY 4

runa_stack *runa_stack_new(size_t initial_capacity) {
    if( initial_capacity < RUNA_STACK_MIN_CAPACITY )
    /* -> */ initial_capacity = RUNA_STACK_MIN_CAPACITY;

    runa_stack *stack = (runa_stack*)malloc(sizeof(runa_stack));
    if(! stack ) return NULL;

    stack->length = 0;
    stack->capacity = initial_capacity;
    stack->values = (void**)malloc(sizeof(void*) * stack->capacity);

    if(! stack->values ) {
        free(stack);
        return NULL;
    }

    return stack;
}

void runa_stack_reserve(runa_stack *stack, size_t new_capacity) {
    if( new_capacity <= stack->capacity ) return;

    void **new_values = (void**)realloc(stack->values, sizeof(void*) * new_capacity);
    if(! new_values ) return;

    stack->values = new_values;
    stack->capacity = new_capacity;
}

void runa_stack_push(runa_stack *stack, void *value) {
    if( stack->length >= stack->capacity ) {
        size_t new_capacity = stack->capacity * 2;
        runa_stack_reserve(stack, new_capacity);
    }

    stack->values[stack->length++] = value;
}

void *runa_stack_pop(runa_stack *stack) {
    if( stack->length == 0 ) return NULL;
    return stack->values[--stack->length];
}

void *runa_stack_peek(runa_stack *stack) {
    if( stack->length == 0 ) return NULL;
    return stack->values[stack->length - 1];
}

void runa_stack_free(runa_stack *stack) {
    if(! stack ) return;
    free(stack->values);
    free(stack);
}

void runa_stack_free_all(runa_stack *stack, void (*destructor)(void *)) {
    if(! stack ) return;

    for( size_t i = 0; i < stack->length && destructor; i++ )
    /* -> */ destructor(stack->values[i]);

    free(stack->values);
    free(stack);
}

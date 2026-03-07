#ifndef RUNA_VECTOR_H
#define RUNA_VECTOR_H

#include <stddef.h>

typedef struct {
    size_t length;
    size_t capacity;
    void **values;
} runa_vector;

runa_vector *runa_vector_new(size_t initial_capacity);

void runa_vector_append(runa_vector *vector, void *value);
void *runa_vector_get(runa_vector *vector, size_t index);
void runa_vector_set(runa_vector *vector, size_t index, void *value);

void runa_vector_remove(runa_vector *vector, size_t index);
void runa_vector_remove_swap(runa_vector *vector, size_t index);

void runa_vector_reserve(runa_vector *vector, size_t new_capacity);
void runa_vector_free(runa_vector *vector);
void runa_vector_free_all(runa_vector *vector, void (*destructor)(void *));

#endif

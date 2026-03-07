#include <stdlib.h>
#include <string.h>
#include "vector.h"

#define RUNA_VECTOR_MIN_CAPACITY 4

runa_vector *runa_vector_new(size_t initial_capacity) {
    if( initial_capacity < RUNA_VECTOR_MIN_CAPACITY ) initial_capacity = RUNA_VECTOR_MIN_CAPACITY;

    runa_vector *vector = malloc(sizeof(runa_vector));
    if(! vector ) return NULL;

    vector->length = 0;
    vector->capacity = initial_capacity;
    vector->values = malloc(sizeof(void*) * vector->capacity);

    if(! vector->values ) {
        free(vector);
        return NULL;
    }

    return vector;
}

void runa_vector_reserve(runa_vector *vector, size_t new_capacity) {
    if( new_capacity <= vector->capacity ) return;

    void **new_values = realloc(
        vector->values,
        sizeof(void*) * new_capacity
    );

    if(! new_values ) return;

    vector->values = new_values;
    vector->capacity = new_capacity;
}

void runa_vector_append(runa_vector *vector, void *value) {
    if( vector->length >= vector->capacity ) {
        size_t new_capacity = vector->capacity * 2;
        runa_vector_reserve(vector, new_capacity);
    }

    vector->values[vector->length++] = value;
}

void *runa_vector_get(runa_vector *vector, size_t index) {
    if( index >= vector->length ) return NULL;
    return vector->values[index];
}

void runa_vector_set(runa_vector *vector, size_t index, void *value) {
    if( index >= vector->length ) return;
    vector->values[index] = value;
}

void runa_vector_remove(runa_vector *vector, size_t index) {
    if( index >= vector->length ) return;
    for( size_t i = index; i < vector->length - 1; i++ ) {
        vector->values[i] = vector->values[i + 1];
    }

    vector->length--;
}

void runa_vector_remove_swap(runa_vector *vector, size_t index) {
    if( index >= vector->length ) return;

    vector->values[index] = vector->values[vector->length - 1];
    vector->length--;
}

void runa_vector_free(runa_vector *vector) {
    if(! vector ) return;
    free(vector->values);
    free(vector);
}

void runa_vector_free_all(runa_vector *vector, void (*destructor)(void *)) {
    if(! vector ) return;
    if( destructor ) {
        for( size_t i = 0; i < vector->length; i++ ) destructor(vector->values[i]);
    }

    free(vector->values);
    free(vector);
}

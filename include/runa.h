#ifndef RUNA_H
#define RUNA_H
#include <stddef.h>
#include <stdbool.h>

#define RUNA_TAG_FIELDS \
    X(runa_string,  "string"), \
    X(runa_integer, "integer"), \
    X(runa_float,   "float"), \
    X(runa_boolean, "boolean"), \
    X(runa_table,   "table"), \
    X(runa_nil,     "nil")

#define X(tag, name) tag
typedef enum {
    RUNA_TAG_FIELDS
} RunaValueTag;
#undef X

typedef union {
    const char* string;
    size_t integer;
    double _float;
    bool boolean;
} RunaValueData;

typedef struct {
    RunaValueTag tag;
    RunaValueData data;
} RunaValueFFI;

typedef struct Runa Runa;
typedef void (*runa_callback)(Runa *);

enum {
    RUNA_FREE_STRING_BY_VALUE
};

#define runa_optional(funcid, cb, x, y)               \
switch (funcid) {                                     \
    case RUNA_FREE_STRING_BY_VALUE: {                 \
        if( ((RunaValueFFI) y).tag != runa_string ) { \
            cb((char*)x);                             \
        }                                             \
    } break;                                          \
}

Runa *runa_start();
void runa_loadfile(Runa *runa, const char *filename);
void runa_free(Runa *runa);
void runa_push_function(Runa *runa, const char *name, runa_callback callback, int argc);
void runa_push_result(Runa *runa, RunaValueFFI value);
RunaValueFFI runa_peek_arg(Runa *runa, int index);
void runa_value_free(RunaValueFFI value);
char* runa_value_to_string(Runa *runa, RunaValueFFI value);
void runa_str_free(char *str);
#endif

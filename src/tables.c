#include <table.h>
#include <stdlib.h>

runa_table* runa_table_new(runa_table_kind kind, void *value, char local) {
    runa_table* table = (runa_table*)malloc(sizeof(runa_table));
    table->kind = kind;
    table->local = local;
    if( value == NULL ) return table;

    if( kind == runa_integer ) table->value.integer = *(int*)value;
    if( kind == runa_string  ) table->value.string  = *(char**)value;
    if( kind == runa_boolean ) table->value.boolean = *(char*)value;
    if( kind == runa_map     ) table->value.hashmap = *(runa_hashmap**)value;
    if( kind == runa_nil     ) table->value.nil     = NULL;

    return table;
}

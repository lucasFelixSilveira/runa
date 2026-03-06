#include "runa.h"
#include <math.h>

void runa_print(Runa *runa) {
    runa_value **args = runa_stack_peek(runa->arguments);
    runa_value argument_one = (*args)[0];


    if( argument_one.kind == runa_string ) {
        char *str = argument_one.value.string;
        printf("%s\n", str);
        return;
    }

    if( argument_one.kind == runa_integer ) {
        int i32 = argument_one.value.integer;
        printf("%d\n", i32);
        return;
    }

    runa_send_error_type(runa, "print", "(string|integer)", runa_value_kind_str(argument_one.kind));
}

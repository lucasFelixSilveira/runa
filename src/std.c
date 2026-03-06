#include "runa.h"
#include <math.h>

void runa_print(Runa *runa) {
    runa_value **args = runa_stack_peek(runa->arguments);
    runa_value argument_one = (*args)[0];


    if( argument_one.kind == runa_string ) {
        char *value = argument_one.value.string;
        printf("%s\n", value);
        return;
    }

    if( argument_one.kind == runa_integer ) {
        int value = argument_one.value.integer;
        printf("%d\n", value);
        return;
    }

    if( argument_one.kind == runa_float ) {
        double value = argument_one.value._float;
        printf("%.15g\n", value);
        return;
    }

    runa_send_error_type(runa, "print", "(string|float|integer)", runa_value_kind_str(argument_one.kind));
}

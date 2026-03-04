#include "runa.h"

void runa_print(Runa *runa) {
    runa_value **args = runa_stack_peek(runa->arguments);
    runa_value argument_one = (*args)[0];
    if( argument_one.kind != runa_string ) {
        runa_send_error_type(runa, "print", "string", runa_value_kind_str(argument_one.kind));
        return;
    }
    char *str = argument_one.value.string;
    printf("%s\n", str);
}

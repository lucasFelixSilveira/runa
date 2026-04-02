#include "runa.h"
#include <math.h>
#include <stdio.h>
#include <unistd.h>

// void mlog2(Runa *runa) {
//     RunaValueFFI val = runa_peek_arg(runa, 0);
//     if( val.tag != runa_integer ) return;
//     runa_push_result(runa, (RunaValueFFI) {
//         .tag = runa_integer,
//         .data.integer = log2(val.data.integer * 8) - 2
//     });
// }

// void rsleep(Runa *runa) {
//     RunaValueFFI val = runa_peek_arg(runa, 0);
//     if( val.tag != runa_integer ) return;
//     sleep(val.data.integer);
// }

void print(Runa *runa) {
    RunaValueFFI val = runa_peek_arg(runa, 0);
    char *str = runa_value_to_string(runa, val);
    printf("%s\n", str);
    runa_optional(RUNA_FREE_STRING_BY_VALUE, runa_str_free, str, val);
    runa_value_free(val);
}

// Callback after push scope in stack
// - Used to define initial scope variables
// - When function is returned all variables are freed
void cap(Runa *runa) {}

int main() {
    Runa *runa = runa_start();

    runa_push_function(runa, "print", (runa_callback)print, 1);

    runa_loadfile(runa, "./main.lua");
    runa_spawn_function(runa, "function_name", (runa_callback)cap);
    runa_free(runa);
    return 0;
}

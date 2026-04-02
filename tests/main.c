#include "runa.h"
#include <stdio.h>
#include <unistd.h>

void print(Runa *runa) {
    RunaValueFFI val = runa_peek_arg(runa, 0);
    char *str = runa_value_to_string(runa, val);
    printf("%s\n", str);
    runa_optional(RUNA_FREE_STRING_BY_VALUE, runa_str_free, str, val);
    runa_value_free(val);
}

void asm_write(Runa *runa) {
    RunaValueFFI val = runa_peek_arg(runa, 0);
    char *str = runa_value_to_string(runa, val);
    printf("%s", str);
    runa_optional(RUNA_FREE_STRING_BY_VALUE, runa_str_free, str, val);
    runa_value_free(val);
}

void asm_writeln(Runa *runa) {
    RunaValueFFI val = runa_peek_arg(runa, 0);
    char *str = runa_value_to_string(runa, val);
    printf("%s\n", str);
    runa_optional(RUNA_FREE_STRING_BY_VALUE, runa_str_free, str, val);
    runa_value_free(val);
}

// Callback after push scope in stack
// - Used to define initial scope variables
// - When function is returned all variables are freed
void cap(Runa *runa) {
    runa_push_field(runa, "kind", make_integer(0));
    runa_push_field(runa, "identifier", make_string("main"));
    runa_push_table(runa, "node", 2);
}

int main() {
    Runa *runa = runa_start();

    runa_push_function(runa, "print", (runa_callback)print, 1);
    runa_push_function(runa, "write", (runa_callback)asm_write, 1);
    runa_push_function(runa, "writeln", (runa_callback)asm_writeln, 1);

    runa_loadfile(runa, "./main.lua");
    RunaValueFFI result = runa_spawn_function(runa, "parse", (runa_callback)cap);
    printf("Result: %ld\n", as_integer(result));

    runa_free(runa);
    return 0;
}

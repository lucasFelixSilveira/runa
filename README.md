<img align="right" src="./assets/runa.png" alt="Runa Logo" width="120px" height="120px">
<br><br>

# 🌙 RUNA Lua interpreter

RUNA is an extremely compact library that interprets Lua 5.x (with an optimized subset) and serves as a hardcore glue between your C, C++, or Rust code and Lua scripts.

Built exclusively with the [Morgana](https://github.com/lucasFelixSilveira/morgana) ecosystem in mind: extensors, yet can be used in any system capable of importing C headers.

# Why use Runa instead of PUC-LUA?

Runa is extremely compact, containing only the functions necessary for Morgana's operation. This means: Less RAM footprint, easier integration into your systems.

Native intercommunication without hacky workarounds — RUNA doesn't rely on FFI (like LuaJIT FFI or libffi) for host bridging. No dynamic calls, automatic marshalling, or foreign call overhead: everything happens in the same address space, with direct bindings via custom C API, sharing the same virtual memory without unnecessary copies.

RUNA isn't a luxury. It's the bare minimum we deserve to avoid being held hostage by non-portable solutions.

# About the syntax

Runa uses the same syntax of PUC-LUA, but you need use perenthesis in function calls, and common native libraries of lua don't works here. Runa has her own std package, with less functions than PUC-LUA. 

```lua
local dev = { name = "Lucas", age = 16, bio = "I am just a developer" }
print("Hello world! My name is " .. dev["name"] .. ", i am " .. dev["age"] .. ' and ' .. dev["bio"])
-- Hello world! My name is Lucas, i am 16 and I am just a developer
```


# How to use

- `Get started` - You can run a lua file using Runa like this 
```c
#include "runa.h"
#include <stdlib.h>

int main() {
    Runa *runa = runa_start(runa);
    runa_loadfile(runa, "main.lua");
    runa_free(runa);
}
```

- `How to call C funcions in Runa (lua)` - You should use `runa_push_function` to push your function in Runa symbol table.
```c
void print(Runa *runa) {
    RunaValueFFI val = runa_peek_arg(runa, 0);
    printf("print from C: %s\n", val.data.string);
    runa_value_free(val);
}

int main() {
    Runa *runa = runa_start();
    runa_push_function(runa, "print", (runa_callback)print, 1);
    runa_loadfile(runa, "main.lua");
    runa_free(runa);
    return 0;
}
```

- `How to use std` - You should use `runa_use_std` function to allocate the std functions into the Runa state.
```c
#include "runa.h"
#include <stdlib.h>

int main() {
    Runa *runa = malloc(sizeof(Runa));
    runa_start(runa);
    runa_loadfile(runa, "main.lua");
    int flags = COMMON_STD | MORGANA_STD;
    runa_use_std(runa, flags);
    runa_free(runa);
}
```

What is the `COMMON_STD` and `MORGANA_STD` flags?

- `COMMON_STD` - Includes common std functions. As `print` and `log10`.
- `MORGANA_STD` - Includes util std functions to Morgana. As `mlog2` and `error`.

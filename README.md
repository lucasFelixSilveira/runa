<img align="right" src="./assets/runa.png" alt="Runa Logo" width="120px" height="120px">
<br><br>

# 🌙 RUNA — Minimal Lua Runtime for Native Systems

RUNA is a compact, low-level Lua 5.x interpreter (optimized subset) designed to act as a zero-overhead bridge between native code (C/C++/Rust) and embedded scripting.

It is built primarily for the [Morgana](https://github.com/lucasFelixSilveira/morgana) ecosystem, but remains fully usable in any environment capable of interfacing with a C ABI.

####

# Why RUNA instead of PUC-Lua?
RUNA is not a full Lua distribution — it is a purpose-built runtime engineered for tight native integration.

## Minimal footprint
- Only the strict subset required for real embedding scenarios
- Reduced memory footprint
- Faster integration, fewer moving parts

## Native intercommunication (no dynamic FFI)

RUNA deliberately avoids **dynamic FFI layers** and any form of runtime foreign call abstraction.
> No libffi. No LuaJIT-style FFI. No runtime call resolution.

Instead, it exposes a **static, ABI-stable C interface** built on direct symbol binding and function pointer dispatch:
- Direct function pointer calls (no indirection layers)
- No trampolines or runtime-generated stubs
- No reflective type inspection
- No dynamic dispatch or call adaptation

All execution occurs **within the same address space**, under a unified memory model:
- Zero context switching
- Predictable, constant-time call boundaries
- Cache-friendly execution paths
- Controlled boundary, leak-free by design

## Crossing the C boundary is explicit, minimal, and strictly controlled:
- Values are converted to a lightweight FFI representation (RunaValueFFI)
- Conversions are deterministic and low-cost
- Temporary allocations may occur, but are short-lived and scoped to the call boundary

Ownership is never ambiguous:
- Any allocation created for C is either explicitly freed, or
- Reclaimed into Rust-managed memory immediately after use

There is no shared ownership across the boundary, and no long-lived foreign allocations.
> If a value crosses the boundary, its lifetime is fully resolved — it is either freed or owned by Rust.
This guarantees:
- **Zero memory leaks by design**
- No hidden heap growth
- No GC interference or unmanaged state
- Fully predictable allocation and deallocation behavior


# Syntax

RUNA follows Lua syntax (PUC-Lua style) with a reduced standard library.

## Differences:
- Function calls require parentheses
- Standard Lua libraries are not included
- Custom minimal `std` is provided

```lua
local dev = { name = "Lucas", age = 16, bio = "I am just a developer" }
print("Hello world! My name is " .. dev["name"] .. ", i am " .. dev["age"] .. ' and ' .. dev["bio"])
-- Hello world! My name is Lucas, i am 16 and I am just a developer
```


## Getting Started
Running a script

```c
int main() {
    Runa *runa = runa_start();
    runa_loadfile(runa, "main.lua");
    runa_free(runa);
}
```

## Exposing native functions
Bind native functions directly into the runtime:

```c
void print(Runa *runa) {
    RunaValueFFI val = runa_peek_arg(runa, 0);
    char *str = runa_value_to_string(val);

    printf("print from C: %s\n", str);

    runa_optional(RUNA_FREE_STRING_BY_VALUE, runa_str_free, str, val);
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

## Using the standard library
```c
int main() {
    Runa *runa = runa_start();

    int flags = COMMON_STD | MORGANA_STD;
    runa_use_std(runa, flags);

    runa_loadfile(runa, "main.lua");
    runa_free(runa);
}
```

### Available flags
- `COMMON_STD` → basic utilities (print, log10, etc.)
- `MORGANA_STD` → extended utilities for Morgana ecosystem

##

RUNA is the bare minimum required to embed Lua without sacrificing control.

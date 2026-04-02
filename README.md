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

## Getting Started

### Running a Script

To execute a Lua script using Runa, you only need to initialize the runtime, load the script file, and then clean up the resources.

### Example

```c
int main() {
    // Initialize Runa runtime
    Runa *runa = runa_start();

    // Execute Lua script
    runa_loadfile(runa, "main.lua");

    // Free runtime resources
    runa_free(runa);

    return 0;
}
```

### Explanation
- `runa_start` initializes the Runa runtime
- `runa_loadfile` loads and executes the specified Lua script
- `runa_free` releases all allocated resources

This is the minimal setup required to run Lua code with Runa.

## Exposing native functions
You can expose native C functions to the Runa runtime, allowing them to be called directly from Lua code.

This is useful for extending Lua with high-performance or system-level functionality.

### Example

```c
void print(Runa *runa) {
    // Get first argument from Lua
    RunaValueFFI val = runa_peek_arg(runa, 0);

    // Convert value to string
    char *str = runa_value_to_string(val);

    printf("print from C: %s\n", str);

    // Free resources if needed
    runa_optional(RUNA_FREE_STRING_BY_VALUE, runa_str_free, str, val);
    runa_value_free(val);
}

int main() {
    Runa *runa = runa_start();

    // Register native function into runtime
    runa_push_function(runa, "print", (runa_callback)print, 1);

    runa_loadfile(runa, "main.lua");
    runa_free(runa);

    return 0;
}
```

- `runa_push_function` registers a C function into the Lua environment
- The third argument (`1`) defines how many arguments the function expects
- `runa_peek_arg` retrieves arguments passed from Lua
- `runa_value_to_string` converts a runtime value into a C string
- `runa_optional` safely frees the string only when required
- `runa_value_free` releases the internal value representation

Once registered, the function can be called in Lua just like a regular function:

```lua
print("Hello from Lua!")
```

This approach lets you seamlessly bridge Lua and C while keeping memory management under control.

## Working with tables in C
To simplify table manipulation in Runa, the internal system uses fake pointers. Since managing these structures manually can be complex and error-prone, Runa provides two helper functions to streamline the process:

- `runa_push_table`
- `runa_push_field`

These functions allow you to construct tables in a structured and safe way without dealing directly with low-level internals.

### Example
```c
int main() {
    Runa *runa = runa_start();

    // Add a field to the current context
    runa_push_field(runa, "name", make_string("Lucas"));

    // Create a table named "dev" with 1 field
    runa_push_table(runa, "dev", 1);

    runa_loadfile(runa, "./main.lua");
    runa_free(runa);
    return 0;
}
```

### Explanation
- `runa_push_field` adds a key-value pair to the current table context.
- `runa_push_table` groups previously pushed fields into a named table.
- 
This abstraction keeps your code clean while avoiding the complexity of handling internal memory structures manually.

## Spawning lua function
To execute (spawn) a Lua function from C using Runa, you first need to have a valid Lua function loaded into the runtime. After that, you can call it using `runa_spawn_function`.

This function allows you to inject arguments and define the execution scope through a callback known as **CAP** (*Callback After Push*).

### What is CAP?
CAP is a callback executed right before the Lua function runs. It is used to define local variables and prepare the function's execution context.

- It runs after the scope is created
- It allows pushing arguments into the function
- All variables defined here are automatically freed after execution

### Example

```c
// CAP = Callback After Push
// Used to define local variables (function arguments)
void cap(Runa *runa) {
    // Push arguments or define variables here
}

int main() {
    Runa *runa = runa_start();

    // Load Lua file containing the target function
    runa_loadfile(runa, "./main.lua");

    // Spawn (call) the Lua function with a CAP callback
    runa_spawn_function(runa, "function_name", (runa_callback)cap);

    runa_free(runa);
    return 0;
}
```

### Explanation
- `runa_spawn_function` looks up and executes a Lua function by name
- The `cap` callback is used to inject arguments into the function scope
- The scope created for the function is temporary and automatically cleaned up after execution

##

RUNA is the bare minimum required to embed Lua without sacrificing control.

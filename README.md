<img align="right" src="./assets/runa.png" alt="Runa Logo" width="120px" height="120px">
<br><br>

# 🌙 RUNA Lua interpreter

RUNA is an extremely compact library that interprets Lua 5.x (with an optimized subset) and serves as a hardcore glue between your C, C++, or Rust code and Lua scripts.

Built exclusively with the Morgana ecosystem in mind: extensors, yet can be used in any system capable of importing C headers.

# Why use Runa instead of PUC-LUA?

Runa is extremely compact, containing only the functions necessary for Morgana's operation. This means: Less RAM footprint, easier integration into your systems.

Native intercommunication without hacky workarounds — RUNA doesn't rely on FFI (like LuaJIT FFI or libffi) for host bridging. No dynamic calls, automatic marshalling, or foreign call overhead: everything happens in the same address space, with direct bindings via custom C API, sharing the same virtual memory without unnecessary copies.

RUNA isn't a luxury. It's the bare minimum we deserve to avoid being held hostage by non-portable solutions.

# About the syntax

Runa uses the same syntax of PUC-LUA, but you need use perenthesis in function calls, and common native libraries of lua don't works here. Runa has her own std package, with less functions than PUC-LUA. 

```lua
print("Hello world running from RUNA!")
```

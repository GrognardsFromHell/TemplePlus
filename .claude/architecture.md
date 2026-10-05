# Architecture: an executable plus the original DLL

- `TemplePlus.exe` is the game executable. It is a standalone 32-bit C++ program, not built on an engine like Unity.
- At startup it loads the original `temple.dll` from the user's ToEE install. The DLL still provides the systems that haven't been rewritten yet. See `Temple/src/dll.cpp`.
- After loading, TemplePlus patches the DLL's code with MinHook so that the old engine calls into TemplePlus replacements.
- So every system is owned by TemplePlus, by `temple.dll`, or is split between them. **The first question for any change is which side owns this logic.**

## Calling into temple.dll

- Addresses are always written as **vanilla addresses** from the DLL's default base `0x10000000`. If the DLL gets loaded at a different base, `temple::Dll` applies the offset automatically. Never hand-compute real addresses.
- `temple::GetRef<0x10AB7550, T>()` and `temple::GetPointer<addr, T>()` return a reference or pointer to a global, rebased (`Temple/include/temple/dll.h`).
- To call a DLL function: `temple::GetRef<int(__cdecl)(objHndl)>(0x100B4D00)(obj);`
- The `AddressTable` subclasses with `rebase(field, 0x10...)` are an older pattern for groups of function pointers.

## Hooking temple.dll (replacing original code)

- Subclass `TempleFix` (`TemplePlus/util/fixes.h`), implement `apply()`, and declare one static instance. It registers itself and is applied at startup.
  - `replaceFunction<Sig>(0x10..., fn)` sends every call to the original function to `fn`. It returns a pointer you can call to reach the original code.
  - `redirectCall`, `writeJump` and `writeNoops` patch single call sites or instructions.
  - `writeHex` and `write` patch raw bytes.
  - `breakRegion` fills a fully replaced function with INT3 so any leftover caller crashes immediately.
- Example: `TemplePlus/xp.cpp` (`XPTableForHighLevels`).
- Add a `/* 0x100B5700 */` comment above a C++ function that reimplements an original one. This convention is used throughout the codebase.
- Calling conventions matter. Many original functions are `__cdecl`, but some are `__thiscall`, `__stdcall` or `__usercall`, where arguments are passed in registers. The usercall ones need asm thunks: search for `__declspec(naked)`.

## Memory layout is a contract

- Structs shared with `temple.dll` must match the original layout byte for byte. Examples: `objHndl` internals, `Dispatcher`, `CondNode`, `D20Actn`, UI widgets, and anything else read through `GetRef`.
  - Don't reorder or resize their fields, and don't add virtual methods.
  - Assert the size with `static_assert(sizeof(X) == 0x..)` or `temple::validate_size`.
- Objects that the DLL may free must come from the DLL-compatible allocator. Derive them from `temple::TempleAlloc`, which works because TemplePlus replaces `malloc`/`free` inside the DLL.
- Changing a signature or layout on the TemplePlus side does not produce a compile error. The DLL side just breaks at runtime.

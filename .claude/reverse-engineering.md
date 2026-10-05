# Reverse-engineering reference

- IDA-decompiled output of `temple.dll`: `TODO: path to temple.c`. This file is **not** in the repo and must never be committed, because it is derived from the commercial binary.
- To understand an address used in TemplePlus code, search `temple.c` for `sub_<ADDR>` (functions) or `dword_<ADDR>`/`unk_<ADDR>` (globals). Treat the decompiled code as strong evidence, not ground truth: types, argument counts and calling conventions are often wrong.
- `TODO: IDA database location / naming status, if relevant`

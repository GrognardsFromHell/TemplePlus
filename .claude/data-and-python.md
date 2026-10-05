# Game data and Python rules

- Classes, feats, spells, races, AI and many combat rules live in Python under `tpdatasrc/tpgamefiles/rules/` (`char_class/`, `feats/`, `spells/`, `races/`, `d20_combat/`, ...).
  - Rules scripts use `from toee import *`.
  - New conditions and feat hooks are Python modules under `tpdatasrc/tpgamefiles/scr/`. They use the `tpdp` module (`TemplePlus/python/python_dispatcher.cpp`), `PythonModifier`, and helpers such as `pymod_utils.py` and `cond_utils.py`.
- The embedded interpreter is Python 2.2 (the version ToEE shipped with). Use old Python 2 syntax only: no f-strings, no `with` statements, no Python 3 features.
- Extended text and enums go in `.mes` files, e.g. `rules/stat_ext.mes`.
- **Edits under `tpdatasrc/` don't take effect until you repack.** Run `tpdatasrc/PackFiles.bat` to rebuild `tpdata/*.dat`. Dev builds load `..\tpdata` relative to the executable.
- Packed `.dat` files are generated and gitignored. Only `tpdata/clearances.dat` is checked in.

## Configuration

- Runtime settings live in `TemplePlus.ini`, mapped to `config.*` in `TemplePlus/config/config.h`.
- Behaviour-changing fixes and house rules are usually behind a config flag so players can opt in or out.
- Vanilla behaviour and Co8 compatibility must keep working. Check `temple::Dll::HasCo8Hooks()` and `modSupport.IsCo8()` where it matters.

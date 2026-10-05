# Creating a new module

Read this when asked to create a new module, or to set up a repo for one. The worked example is `C:\Users\anato\source\repos\github.com\Anatoliy.Savchak\toee.zmod.kots`.

## How TemplePlus loads a module (from the code)

- **Which module loads** is set by `defaultModule` in `TemplePlus.ini` (`config.defaultModule`, default `ToEE`).
  - The ini lives in the user's Saved Games folder: `%USERPROFILE%\Saved Games\TemplePlus\TemplePlus.ini` (`util/folderutils.cpp`). `TemplePlus.log` is there too.
  - The Configurator can also set it.
- A module named `<name>` consists of two parts under `<ToEE>\modules\`. Each part can be a loose folder, a `.dat` archive, or both.

  | Part | Paths | Loaded | Typical contents |
  |---|---|---|---|
  | **Core** | `<name>_core\`, `<name>_core.dat` | Early, at TIG startup, right after `tpdata` (`tig/tig_startup.cpp`) | `scr/`, `dlg/`, `mes/`, `oemes/`, `rules/` (protos, classes, feats, spells), `tpmes/`, `tprules/`, `templeplus/lib`, `art/`, `sound/` |
  | **Map data** | `<name>\`, `<name>.dat` | Later, in `GameSystems::LoadModule` (`gamesystems/gamesystems.cpp`) | `maps/` (MOBs, ground art), `rules/MapList.mes`, lighting `.mes` files, `WMap_Rnd.mes`, `townmap/`, plus `Save/` and `TIGCache/` at runtime |

  - The module archive's GUID is used to preprocess the map MOB files.
  - The game writes save data under the module folder. Keep `Save/`, `TIGCache/` and `players/` out of git.
- **The `zmod` naming convention.** If the module name contains `zmod`, `modSupport.IsZMOD()` returns true (`mod_support.cpp`). This turns on non-ToEE behaviour in several places:
  - AI doesn't auto-wield the best weapon and doesn't pick heal spells (`ai.cpp`).
  - Trade goods sell at their full worth (`inventory.cpp`).
  - Item tooltips and the character UI change (`ui/ui_char.cpp`).
  - In Python, `tpdp.is_temple_module()` returns false and `get_config("iszmod")` returns true.

  Name new engine-style modules `zmod_<name>`. Treat these `IsZMOD()` branches as a stopgap: for new behaviour, prefer a dedicated config flag or a Python hook (see `purpose.md`).

## Steps

1. **Create a repo per module**, named `toee.zmod.<name>`, with the same layout as kots:
   ```
   src/zmod_<name>/        map data part
   src/zmod_<name>_core/   core part (+ scr.builtin/ API stubs, not packed)
   settings/map_module.cmd junctions into <ToEE>\modules\
   pack/                   TioPack.exe + tio.dll + zlib, packing script
   docs/                   design notes, to_improve.md (engine gaps found)
   CLAUDE.md
   ```
   Copy `.gitignore` from kots, and also ignore the runtime folders `Save/`, `TIGCache/`, `players/` and the packed `*.dat` files.
2. **Start from kots, not from scratch.** Copy kots's `rules/`, `mes/`, `tpmes/` and `tprules/` skeleton, then strip out the kots-specific maps, dialogue and scripts.
   - The minimal set of files a module needs to boot hasn't been documented yet. If you work it out, record it here.
   - `TODO: minimal bootable module file list`
3. **Link the module into the game** for development. Use directory junctions so the game reads the repo directly and no repacking is needed:
   ```
   mklink /j "D:\Games\Temple of Elemental Evil\modules\zmod_<name>"      <repo>\src\zmod_<name>
   mklink /j "D:\Games\Temple of Elemental Evil\modules\zmod_<name>_core" <repo>\src\zmod_<name>_core
   ```
4. **Select it** by setting `defaultModule = zmod_<name>` in `TemplePlus.ini`.
5. **Add API stubs.** Copy `scr.builtin/` (`toee.py`, `tpdp.py`, ...) for IDE completion.
   - They are hand-written and incomplete: the kots `toee.py` has 144 of the 251 `PyObjHandle` methods.
   - The source of truth is this repo's `TemplePlus/python/python_object.cpp`, `python_game.cpp` and `python_dispatcher.cpp`. Prefer regenerating the stubs from those files over hand-editing them.
6. **Package releases.** Pack each part with `TioPack.exe /R` into `<name>.dat` and `<name>_core.dat`. The loader looks for exactly `modules\<defaultModule>.dat` and `modules\<defaultModule>_core.dat`.
   - The kots script currently writes `zmod_kots_001.dat` and `zmod_kots_001_core.dat`. Those only load if `defaultModule = zmod_kots_001`, or if the files are renamed.
7. **Engine gaps go into TemplePlus, not the module.** When the module needs behaviour the engine lacks:
   - Log it in the module's `docs/to_improve.md`.
   - Implement it here, generically: a Python API, a config flag or a data-driven change, never a check for this one module.

## Module repo `CLAUDE.md` template

```markdown
# zmod_<name>

New ToEE module running on TemplePlus (engine: C:/Users/anato/source/repos/github.com/TemplePlus).

- `src/zmod_<name>_core` = core part (scripts, dialogue, rules, mes), `src/zmod_<name>` = maps.
- Dev: junctions via `settings/map_module.cmd`, `defaultModule = zmod_<name>` in
  `%USERPROFILE%\Saved Games\TemplePlus\TemplePlus.ini`. Release: `pack/` scripts.
- Python is the embedded ToEE interpreter (Python 2.2 syntax). `scr.builtin/*.py` are IDE stubs,
  possibly stale; the real API is in TemplePlus `TemplePlus/python/python_*.cpp`. Check there
  before concluding a method doesn't exist.
- Engine bugs/features: log in `docs/to_improve.md`; fix in TemplePlus, generically.

@C:/Users/anato/source/repos/github.com/TemplePlus/.claude/purpose.md
@C:/Users/anato/source/repos/github.com/TemplePlus/.claude/data-and-python.md
@C:/Users/anato/source/repos/github.com/TemplePlus/.claude/new-module.md
```

## Session setup

- **Module content work:** open the module repo as primary and add the TemplePlus folder as a secondary folder.
- **Engine work:** open TemplePlus as primary and add the module repo as a secondary folder, to use as a test case.
- An added folder's `CLAUDE.md` isn't loaded unless `CLAUDE_CODE_ADDITIONAL_DIRECTORIES_CLAUDE_MD=1` is set. That's why the template imports specific TemplePlus notes with `@` instead.

# Module strategy: TemplePlusAM, the module SDK and the scripting API

Planning notes from 2026-10-06. Nothing here is built yet. Read this before working on the fork's direction, the shared Python framework, module persistence, map-level scripting or trigger zones.

## Direction

- This fork may become **TemplePlusAM**: TemplePlus plus a built-in SDK for new modules.
  - Upstream (GrognardsFromHell/TemplePlus) has been quiet since 2026-02-22.
  - In 2025 it had about 180 commits, mostly from Dan Doel (dolio) and Doug.
  - The hope is that new modules bring players and developers back, to this fork.
- **Keep the internal names.** That means the exe, solution, namespaces, and the GitHub fork with its history. Brand only what users see: Configurator, window title, README, releases. Returning developers should recognise it as TemplePlus continued.
- **Vanilla ToEE and Co8 must keep working.** The Co8 community is the first audience.
- **Licence:** MIT, Copyright 2015 Circle of Eight. Keep the notice and add a line for this fork.
- **What brings people back:**
  1. released, playable modules
  2. packaged releases plus a Co8 forum thread
  3. a public module-making guide (much of `new-module.md`)
  4. fixing upstream's open issues
  5. then asking the old maintainers for a README link or a maintainership transfer

## The existing Python framework

- **Where it lives:** module repos `toee.zmod.kots`, `toee.zmod.barrow` and `toee.zmod.iwd2`, under `src/zmod_<name>_core/scr/`.
- **Size:** about 7.7k lines of core (`ctrl_*`, `utils_*`, `const_toee`, `startup_zmod`), plus `const_proto_*`, `ait/` and `tpModifiers/`.
- **It's copy-pasted into each module, and the copies have drifted.** Compared with kots:
  - `ctrl_daemon2.py` is identical everywhere.
  - `ctrl_behaviour.py` differs by 111 lines in barrow and 328 in iwd2.
  - `utils_npc.py`, `const_toee.py`, `utils_obj.py` and `startup_zmod.py` differ too.

### What it really is

It's a **scripting runtime** built in Python, because the engine had no such concepts. It has five parts:

1. **Map daemon.**
   - An invisible NPC is placed in WorldEd and given a script number, e.g. `py06615_daemon_orc_fort.py`.
   - Its `san_new_map`, `san_first_heartbeat` and `san_heartbeat` handlers forward to a `CtrlDaemon2` subclass.
   - That class spawns encounters and holds persistent fields.
2. **Controllers.**
   - `CtrlBehaviour` subclasses are created alongside dynamically spawned NPCs (`create_obj_and_class`) and stored under `storage.data["ctrl"]`.
   - Numbered script files hold `san_*` functions that only look up the controller and forward the call.
3. **Encounters.**
   - `create_npc_at` and `monster_setup` register each spawn in a `MonsterInfo` registry by encounter and code name.
   - Monsters move through `reveal_monster` (clears `OF_DONTDRAW`) and then `activate_monster` (clears `ONF_NO_ATTACK`, sets `ONF_KOS`).
4. **Prompters.**
   - `py06122_cormyr_prompter.py` is an invisible *critter* whose heartbeat polls `obj_list_range` plus line of sight.
   - When a PC comes near, it starts a dialog, floats a line or runs `san_bust`, then destroys itself.
   - This drives proximity reveals and encounter descriptions.
5. **Persistence.** `utils_storage.py`, see below.

There's also tactics (`CtrlBehaviourAI`, `ait/`, `utils_npc_spells_tactics.py`). It's built on the `d20_ai.combat_strategy` hook, and it often fails because of pathfinding (`pathfinding.md`).

### Bugs found while reading it

- `do_san_*` in `ctrl_daemon2.py` uses `if (attachee.map != map_id): toee.RUN_DEFAULT` with no `return`, so the guard does nothing.
- In `utils_storage.py`, `obj_storage_by_id`, `obj_storage_by_alias` and `ca` assert on an undefined `obj`. They only work because asserts are stripped (see `data-and-python.md`).
- `instantinate_by_eval` is called without the `Storage.` prefix.
- kots `py06615_daemon_orc_fort.py`:
  - `place_monsters_r21` is defined twice, so the executioner never spawns.
  - `place_encounters_initial` calls it twice, so Icarus's group spawns twice.
  - `place_monsters_r12` is never called.

## `utils_storage.py` assessment

**How it works.**
- Each object gets an `ObjectStorage`, keyed by its GUID, holding a dict of Python objects.
- It's serialized to JSON, one file per object, in `modules\<mod>\save\Current\dSlot`.
- It runs from `templeplus.savehook.save`, which is before the save archive is built (`gamesystems/gamelib_save.cpp`). So the files end up inside each save. Keep that part.

**Problems.**
- Save and load each run twice: also via `_co8init` through `post_save` and `post_load`. The second load replaces every controller instance.
- Reset only happens from `py00406Game_Portal.py`.
- Entries for destroyed objects are never removed.
- Each save deletes all files first, then writes them one by one, so a failure part-way loses data. Exceptions are swallowed, so the save still reports success.
- The JSON round-trip changes the data:
  - shared references come back as separate copies
  - reference cycles recurse forever
  - saving writes `_isofclass` and `_isofmodule` into live objects
  - renaming a class or module breaks old saves
  - on load it `__import__`s module names read from the save
- It has a hardcoded `"zmod_coe"` fallback and cwd-relative paths.

**Proposal: C++ owns the lifecycle, Python owns the serialization.**
- **C++ side:** a new game system.
  - Hooks save, load, new game and reset, once each.
  - Writes a single versioned file, `Save\Current\modstore.dat`, before the archive is built.
  - It's a separate file, not a new section in the save stream, so saves still load in stock builds.
  - Failures are visible and logged.
  - Entries are keyed by `ObjectId`, plus named and aliased entries. Orphans are pruned at save time.
  - Python API such as `obj.module_data` and `game.module_data`, viewable in the debug UI.
- **Python side:**
  - `cPickle` protocol 2, run from C++, with a restricted unpickler and a rename table.
  - A `utils_storage` compatibility facade.
  - A one-time import of old `dSlot` JSON saves.
- **Reuse:** it could also back Co8's `Co8PersistentData` later.

## Three layers

| Layer | Owns | Lives in |
|---|---|---|
| Engine (C++) | events, timing, save lifecycle, geometry and pathing, UI | TemplePlus |
| SDK (Python) | how adventures work: controllers, encounters, tactics policy | TemplePlusAM, a namespaced package (e.g. `am/`) loaded only by modules that opt in |
| Content | anything with a place, NPC or plot name | the module repo |

**Rule:** it's engine work if it needs engine events, the save file, per-frame performance or UI. It's SDK if it describes *how* an adventure works. It's content if it names *what*.

## Where each part goes

| Framework part | Destination | Engine work |
|---|---|---|
| Map daemon NPC | **Engine** | Map script binding in data (map ID → Python module or class). Calls `on_enter(first)`, `on_leave`, `on_load`, a game-time `tick`, and map-wide events: critter dying, portal used, combat start and end. Map state goes in the storage above. |
| Controllers plus `san_*` trampolines | **Engine** mechanism, **SDK** base classes | An object holds a persisted controller reference (`obj.controller = CtrlTiger()`) and events go straight to its methods. Numeric script IDs become optional. |
| Encounters (spawn, reveal, activate) | **SDK** | Engine only removes workarounds: real custom names (replacing `make_custom_name` and the description-field hack), spawning with offset and rotation in one call, a proper dormant state. |
| Hand-written placement code | **Data** | Spawn points, prompters and zones go on a Tiled object layer, the same `battlegrid.tmj` as `battle-grid.md`. Python keeps only behaviour. |
| Prompters | **Engine** | Trigger zones: a circle or polygon, a condition (PC enters, line of sight, not in combat, once), a Python callback. Event-driven from movement, no polling, no critter. Plus a narration UI: text or dialog with no speaker NPC. |
| Tactics | Split | **Engine:** pathfinding fixes and query APIs (reach and cost this turn, paths that avoid AoO, flanking squares, threatened squares). **Python:** decision policy as an opt-in "AI v2" under T+ `d20_ai`. |
| Persistence | **Engine** | See above. |
| `zmod_templeplus_config_apply()` | **Engine** | A module manifest (`module.ini`): config overrides, default PC conditions, framework version and dependencies. It could replace the `IsZMOD()` name hack. |
| `const_proto_*.py` | **Tooling** | Generate them from `protos.tab`. |

The storage, controller binding, map scripts and trigger zones are **one feature**, the "module scripting API". Design them together, and build storage first.

## Rough effort

These are ranges in focused working days with Claude doing most of the coding. They're unvalidated guesses for sizing, not commitments. Anything touching code owned by `temple.dll` (movement events, UI) may take longer.

| # | Item | Days |
|---|---|---|
| 0 | Line-ending cleanup. The working tree has 25 files with ±40,039 lines of churn. | 0.5 |
| 1 | Three-way merge of the framework copies into one namespaced package | 2–4 |
| 2 | Framework layer loading: `am.dat`, or generic module dependencies | 1–2 |
| 3 | Module manifest (`module.ini`) | 1–2 |
| 4 | Storage game system, plus facade and JSON importer | 3–5 |
| 5 | Controller binding | 3–5 |
| 6 | Map script binding | 3–5 |
| 7 | Trigger zones | 4–7 |
| 8 | Narration UI | 2–4 |
| 9 | Spawn API gaps: custom names, dormant state | 2–4 |
| 10 | Tiled object-layer loader, shared with the battle grid | 3–5 |
| 11 | Pathfinding fixes plus tactics query APIs | 10–20 |
| 12 | Port AI v2 into T+ `d20_ai` | 5–10 |
| 13 | Migrate kots onto all of the above | 5–10 |

- **Engine scripting API (items 4–8):** about 15–26 days.
- **Total:** about 45–85 days.

Items 0–4 give a working shared SDK early. The modules keep working throughout, because of the `utils_storage` facade.

# Temple+ 1.1.0

The first 1.1 release. It adds a new prestige class, data-driven module support for total conversions and new campaigns (Icewind Dale and the Dancing Hut are the first users), DM/debug tools and a batch of crash fixes. It also includes all upstream changes since 1.0.98.

## New class: Dragon Disciple
- The Dragon Disciple prestige class (PHB/DMG 3.5) is now selectable, with ability boosts, natural armor, bonus spells, draconic heritage, claws and bite, breath weapon (cone), blindsense, wings (cosmetic, vanilla addmesh) and dragon apotheosis.
- Prerequisites are adapted to ToEE. Knowledge (arcana) and Draconic are dropped, and character level 5 stands in for the skill ranks, as with Mystic Theurge.
- Help topics and the class selection description are included.
- **Natural weapon blend (opt-in):** extra natural attacks (claws, bite) alongside manufactured weapons, with free-hand rules (buckler, Prefer One Handed Wield, no stacking of 1.5x Str and a claw), monk flurry interaction, and support in the Python combat path.

## Module support (total conversions / new campaigns)
- **Data-driven new game start map:** a `MapList.mes` `NEW_GAME_START` flag, NewGameX/Y, and a Python hook (`templeplus/new_game.py`). Modules no longer need a patched `temple.dll` vignette table. See `docs/modding-start-map.md`.
- **Data-driven worldmap:** `rules/worldmap_areas.mes` covers areas, multi-entrance towns and worldmap access.
- **co8fixes.dat** is only registered for Co8 New Content (or standalone Co8 without TFE-X), so it no longer leaks into other modules.
- **Map script tick:** `map_tick.on_tick(mapId, elapsedMs)` runs on the animation game clock and pauses with menus. It is inert without a script.
- **Info regions:** `rules/info_regions/<map>.json` defines clickable polygons that show text in a speech bubble over the leader, with a magnifier hover cursor.
- **Optional IE-style dialogue skins:** per module or per dialog, with NPC side portraits and portraits in the classic log layout.
- **Dialogue reply paging:** all valid PC replies are shown, and more than 5 are paged with More.../Back.
- **Door/tile hooks:** `game.tile_flags_get/set` and ground overlays.
- **Map alerts:** per-map ally alert distance (see `docs/modding-map-alerts.md`), container art / loot polygons, and Python queries for combat allegiances ("Party Combat Ally", "Can Disengage From Party", "Combat Practice Target", "Dies at Zero HP").

## Movies
- Bink movies and slides can be skipped with Space, Enter, Esc or a mouse click.
- Slides end after their sound (minimum 3 s) even when the sound fails to play, instead of waiting for a key forever. The failure reason is logged.

## DM / debug tools
- **DM sector tile editor:** brush size, subtile (1/3 tile) resolution, and painting flyover/cover with toggle/set/clear modes. Edits can be saved (JSON per map under `Saved Games\TemplePlus\dm_tile_edits\<module>`) and are re-applied on sector load. Door cells set by scripts are protected.
- **Clipping-mesh debug draw:** `debug.clipping(on)` / `config.debugClipping`.
- **Path node bake helpers** for map conversions, through the existing debug hooks (nearest-first neighbour recalc, two-way links, nominal PC radius).
- **Anomaly notices for playtesters** (ini `anomalyNotices`): a generic C++/Python API, with a persistent radius/height anomaly log stored in the save.
- **Object handle validator** for inventory corruption (ini `debugObjHandles`, debug only).
- **Alert debug overlay** (`debug.alerts(1)`) and a goal overlay (`tpgui.set_autoplayer_goals`).

## Autoplayer support
- The ToEE autoplayer can now run on stock Temple+, so players can watch it play. Install the [ToEE autoplayer](https://github.com/DudeMcDude/toee_autoplayer) scripts and launch with the startup automation variables. The autoplayer game system only starts when `templeplus/autoplayer.py` is installed.
- New Python APIs for UI automation and scripting:
  - **game:** `is_ingame`, `time_played`, `time_session`, `char_ui_page`, `mouse_scroll`, `mouse_set_button_state`, `scroll_to(loc, smooth)`, and `mouse_move_to((x, y))`. `keypress` now also sends a character event.
  - **gamedialog** (new module): `is_engaged()` and `get_current_state()`, returning the dialog state (script id, line, replies, reply opcode/effect/skill, NPC reply id).
  - **tpgui:** widget lookup and inspection (`_get_active_windows`, `_get_legacy_*`, `_get_adv_widget_*`, widget names, rendered text, scrollbars, button state), `game_is_focused`, `map_from_scene`, `world_to_screen`.
  - **Other:** `tpactions.cur_seq_reset`, `obj.is_performing()`, `obj.ai_strategy_execute()`, `SpellEntry.ai_spell_type`, timestamp `delta_ms`/`add_ms`/comparison, and `logbook.set_key_entry_need_to_notify`.
- Enter now confirms the logbook key entry dialog.

## Startup automation
- `TP_AUTOLOAD=<save>` loads a savegame at startup, skipping the main menu, and `TP_AUTOEXEC=<command>` runs a console/Python command. Both do nothing unless set. They are used by the ToEE autoplayer and for repeatable playtests; see `docs/startup-automation.md`.

## Bug fixes
- Fixed a stale-pose object radius/height and an uncached zero radius.
- Guarded the cloth simulation against non-finite or runaway vertices (ini `clothSimGuards`).
- Fixed slowdowns when `float_line` is spammed.
- Action sequences check for exhausted path slots, roll back partially built sequences, and reserve space for readied interrupts. This fixes crashes in large fights.
- Null-path guard in AI ImprovePosition, and a pathfinding/raycast fix.
- Combat ends when the remaining nearby combatants can disengage.
- CI locates Visual Studio via vswhere and builds on windows-2022.

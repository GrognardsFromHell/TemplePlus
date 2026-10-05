# Purpose and history

## How the project came about

1. **2003: Troika Games releases *The Temple of Elemental Evil* (ToEE).** It is a turn-based D&D 3.5 CRPG built on Troika's own engine. Much of the game logic and content is scripted in embedded Python, which made it unusually moddable.
2. **2000s–2010s: Circle of Eight (Co8, https://co8.org/community/).** Fans used that Python integration to fix bugs and add rules, content and whole modules (the Co8 Modpack, KotB, and others). They were limited to what the engine exposed to Python.
3. **The debug symbols leak.** The Polish retail release of ToEE shipped with unstripped debug symbols. That made serious reverse engineering of `temple.dll` practical: function names, structs and globals.
4. **2010s: TemplePlus.** C++ enthusiasts started rewriting the engine side on top of that work: rendering, UI, D20 rules, combat, AI, the character system and mod infrastructure. **The goal was to rewrite the technical side and the rules, not the content.** The original ToEE campaign and Co8 content still have to work.

## This fork's goal: ToEE as an engine

The maintainer of this checkout goes one step further. They want to **use ToEE/TemplePlus as an engine for entirely new modules**, not just as a better way to play the original campaign.

- Example module: `C:\Users\anato\source\repos\github.com\Anatoliy.Savchak\toee.zmod.kots`
  - `src/zmod_kots` holds maps, art, mes and rules.
  - `src/zmod_kots_core` holds dlg, scr, rules, `tpmes`/`tprules`/`templeplus` overrides and sound.
  - `settings/map_module.cmd` links both folders into `<ToEE>\modules\` with directory junctions.
  - `pack/tio_pack_module.bat` packs them into `.dat` archives with `TioPack.exe`.
  - `docs/to_improve.md` lists engine bugs and missing features found while building the module. Many of them are TemplePlus-side work.

## What this means for changes

- **Prefer generic, data-driven or Python-exposed mechanisms over hardcoded ToEE content.** That means new hooks, `tpdp` API, config, `.mes`/`.tab`-driven behaviour, and not assuming ToEE maps, protos, NPCs or quest flags. Upstream commit #910, "Make builtin race/class conditions more moddable", is in this spirit.
- When engine code special-cases a vanilla map, proto or script ID, treat that as a candidate for generalising. Don't extend the special case.
- Upstream compatibility still matters: the vanilla ToEE campaign and Co8 must keep working, and changes should stay upstreamable to GrognardsFromHell/TemplePlus where possible.
- A bug report may come from a new module rather than from ToEE. Ask or check which content is involved before assuming vanilla data.

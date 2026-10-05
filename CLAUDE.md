# TemplePlus

TemplePlus is an open-source, partial rewrite of *Temple of Elemental Evil* (ToEE, 2003).

`TemplePlus.exe` is a standalone 32-bit game executable. It loads the original `temple.dll` for systems that haven't been rewritten yet, and patches the DLL so it calls into the new code. Before changing anything, work out which side owns the logic involved.

This checkout's goal is to use ToEE/TemplePlus as an **engine for new modules**, not only to improve the original campaign. See `purpose.md`.

Detailed notes live in `.claude/` and are imported below:

@.claude/purpose.md
@.claude/architecture.md
@.claude/reverse-engineering.md
@.claude/data-and-python.md
@.claude/build-and-run.md

When creating a new module or setting up a module repo, read `.claude/new-module.md` first. It isn't imported because it's only needed for that task.

## Repository layout

| Path | Contents |
|---|---|
| `TemplePlus/` | Main executable. Game systems (`gamesystems/`, `d20*`, `critter`, `combat`, ...), UI (`ui/`), Python bridge (`python/`), DLL patches (`fixes/`, plus `TempleFix` classes throughout). |
| `Temple/` | Static library that loads the DLL: address rebasing, `GetRef`, VFS, sound, movies. |
| `Infrastructure/` | Shared utilities: graphics device, logging, INI, VFS, debug UI (imgui). |
| `ParticleSystems/`, `Tools/` | Particle system library, MDF/particle editors and previewers. |
| `Configurator/` | WPF launcher/config UI that writes `TemplePlus.ini`. |
| `Tests/` | Infrastructure and particle system tests. Gameplay logic has mostly no automated tests. |
| `tpdatasrc/` | Source for game data: Python rules, `.mes` text files, `.tab` protos, art. Split into `tpgamefiles` (core TemplePlus), `co8fixes`/`co8infra` (Circle of Eight mod compatibility), `kotbfixes`, `palcov`. |
| `tpdata/` | Packed `.dat` archives and loose data that the game loads at runtime. |

## Conventions

- Match the surrounding style. The code mixes old reverse-engineered names with newer modern C++. Prefer the existing system singletons (`critterSys`, `d20Sys`, `combatSys`, `party`, `config`, ...) over new globals.
- Use the `logger` (spdlog) for diagnostics.
- When fixing a rule bug, keep vanilla or Co8 behaviour available when players might rely on it.

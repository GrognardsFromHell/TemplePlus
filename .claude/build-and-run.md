# Building and running

- Visual Studio 2022, Win32 (32-bit only). Open `TemplePlus.sln` and build Debug or Release.
- Dependencies: unzip the `dependencies.7z` release from GrognardsFromHell/Dependencies into the repo root. CI uses v1.19 (`.github/workflows/ci.yml`).
- Command line: run `msbuild TemplePlus.sln /p:Configuration=Release /p:Platform=Win32` from a VS x86 developer prompt.
- Running requires a ToEE install that contains `temple.dll`. The local install is `D:\Games\Temple of Elemental Evil`: the GOG version with Co8 Modpack 8.1.0 Standard and TFE-X.
  - Modules live in `<ToEE>\modules\`: `ToEE` (the original game), `zmod_kots` and `zmod_kots_core` (junctions to the module repo).
  - `TODO: how TemplePlus is pointed at the install and selects a module (TemplePlus.ini / Configurator)`
- Testing is mostly manual and in-game. `TODO: preferred test saves, debug console usage, cheats`

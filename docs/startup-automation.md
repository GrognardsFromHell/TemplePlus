# Startup automation: TP_AUTOLOAD / TP_AUTOEXEC

Temple+ reads two optional environment variables at startup. This happens after `Startup.txt` runs and before the main loop begins. Neither does anything when unset, so a normal launch is unaffected.

| Variable | Effect |
|---|---|
| `TP_AUTOLOAD=<save name>` | Skips the main menu and loads the named savegame: the archive's base name in the `save` folder, e.g. `SlotAuto` for `SlotAuto.tfai`/`.tfaf`. It waits until the map has finished loading. |
| `TP_AUTOEXEC=<command>` | Runs one console command or Python statement, the same as typing it in the console, after the optional autoload. |

They are used by the [ToEE autoplayer](https://github.com/DudeMcDude/toee_autoplayer), so players can watch it play and testers can reproduce a run:

```bat
set "TP_AUTOLOAD=SlotAuto"
set "TP_AUTOEXEC=from templeplus import autoplayer; autoplayer.play_new_game()"
TemplePlus.exe
```

They are also handy for repeatable playtests and bug reports: "launch with `TP_AUTOLOAD=bug123` and it happens at once."

Notes:
- The command runs with the same rights as the console. It is a convenience for whoever starts the game, not a sandbox.
- Unset the variables, or start the game normally, to get the main menu again.
- Both steps are logged as `[TP_AUTOLOAD]` / `[TP_AUTOEXEC]` in `TemplePlus.log`.

# New game start map (modules)

Temple+ lets a module choose where a new game starts without patching `temple.dll`
(previously done with TOEEWB by editing the alignment vignette table / start map global).

## Precedence
1. **Python hook**: `scr\start_location.py` in the module, function
   `get_start_location(alignment, default_map)`. Return `None` (no override), a map id,
   or `(map_id, x, y)`. Called through `templeplus.new_game`.
2. **`Rules\MapList.mes`**: the map whose entry has `Flag: NEW_GAME_START`. The party starts at
   `NewGameX`/`NewGameY` if given, otherwise at the map's MapList start X/Y.
3. **Engine default** (unchanged behaviour): the engine start-map global if set, otherwise the
   alignment vignette map (vanilla: vignettes 5096-5104; Co8 / patched DLLs: whatever their
   table holds, e.g. 5001).

If none of 1/2 apply, nothing changes. Invalid map ids are logged and ignored.
The log states which source was used: `New game: starting on map ...`.

## MapList.mes keys
```
// Flag: NEW_GAME_START -- new games start here (only one map; replaces the alignment vignettes)
// NewGameX: <x>, NewGameY: <y> -- optional new-game start tile (otherwise the map's start X/Y)
{5001}{Map01-hommlet-exterior, 623, 421, Flag: NEW_GAME_START, NewGameX: 610, NewGameY: 430, ...}
```
Why not `Type: START_MAP`? Vanilla's MapList already marks Vignette-Chaotic (5104) as
`START_MAP`, and the engine uses that type as the map opened at module load when there is no
`SHOPPING_MAP`. Honouring it would send every vanilla party to the chaotic vignette, so the
existing type keeps its engine meaning and a new opt-in flag is used instead.

## Other related MapList keys (existing)
- `Area: <n>` -- index in `mes\area.mes`; overrides the engine's hardcoded map -> area table.
- `Type: SHOPPING_MAP` / `TUTORIAL_MAP`, `Movie: <id>`, `WorldMap: <n>`,
  `Flag: DAYNIGHT_XFER | OUTDOOR | UNFOGGED | BEDREST`.

## Python example (`scr\start_location.py`)
```python
def get_start_location(alignment, default_map):
	if default_map in range(5096, 5105):   # skip vignettes
		return (5001, 623, 421)
	return None
```

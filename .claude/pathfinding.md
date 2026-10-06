# Pathfinding: how it works and known problems

Audit of the TemplePlus pathfinder, written 2026-10-06. Findings come from reading the code, not from profiling. Line numbers are from that date and may drift.

Files: `TemplePlus/pathfinding.{h,cpp}`, `TemplePlus/path_node.{h,cpp}` (path nodes and clearance), `TemplePlus/raycast.cpp`. Main callers: `ActionSequenceSystem::MoveSequenceParse` (`action_sequence.cpp`), `ai.cpp`, `python_object.cpp`.

## How a path is found

- `FindPath` (replaces `0x10043070`):
  1. `PathInit`. With `PQF_TARGET_OBJ | PQF_ADJUST_RADIUS`, it adds both radii to `distanceToTargetMin` and `tolRadius`.
  2. `GetAlternativeTargetLocation`: if the destination is blocked, search nearby for a free spot.
  3. `TargetSurrounded`.
  4. Path cache lookup.
  5. Path nodes (`FindPathUsingNodes`) or a direct search (`FindPathSansNodes`).
- `FindPathSansNodes` tries a straight line first, then an A* search on a 160 × 160 **subtile** grid:
  - `FindPathShortDistanceSansTarget` for a location.
  - `FindPathShortDistanceAdjRadius` for "get within reach of a target".
- Blocking during the search:
  - **With `clearance.bin`:** a per-subtile clearance map (`PathNodeSys::clearanceData`), plus a `ProximityList` of nearby creatures, doors and containers.
  - **Without it:** a raycast for every step.
- `PathNodesAddByDirections` turns the 8-direction steps into waypoints by raycasting ("smoothing").
- `FindPathUsingNodes` routes through hand-placed path nodes (`FindPathBetweenNodes`), then runs a direct search between each pair of nodes.

## Root cause of bad combat positioning (the main UX problem)

D&D assumes 5-ft squares: 8 Medium creatures fit around one Medium creature. ToEE uses continuous positions and circles instead:

- **The footprint is the model radius, not the D&D size.** `Objects::GetRadius` (`obj.cpp:445`) comes from the proto or the model's bounding box, typically 10–17 in, where a 5-ft square implies 30 in. Different checks also scale it differently:
  - `PathDestIsClear` uses 1.0 × the radius.
  - The search's creature check uses 0.7 ×.
  - Smoothing raycasts use 0.7 ×.
  - The clearance check uses 0.85 × when the radius is over 12 in.
- **Where an attacker stops is arbitrary.**
  - A melee approach (`action_sequence.cpp:1137-1144`) accepts any center-to-center distance in `[rT + rM + min, rT + rM + reach·12 − 12.7]`.
  - `FindPathShortDistanceAdjRadius` stops at the first subtile in that ring it reaches, usually the outer edge on the approach side (`pathfinding.cpp:1287`).
  - Attackers end up at arbitrary angles and distances. They leave gaps slightly too small for another body, so the target can't be surrounded.
- **The rules are distance- and angle-based.**
  - Reach is distance minus radius (`combat.cpp:349-410`).
  - Flanking is a 120–240° angle test, and the AI's `Flank` picks a spot by angle (`ai.cpp:3128-3165`).
  - Snapping positions alone would not help: diagonal neighbours (84.85 in apart) fail the reach check with ~12-in radii.

The planned fix is the battle grid; see `battle-grid.md`.

## Bugs

- **NPC search throttle shared by all NPCs.** `static` counters (`pathfinding.cpp:1146`, `:2099-2126`) fail every search silently after 10 attempts per second (50 with clearance data, always 10 for the reach search) or 250 ms of search time. A single path through nodes runs one search per node pair. This mostly affects real-time play; in turn-based combat it rarely triggers.
- **Ring searches only cover part of each ring.** `TargetSurrounded` (`:677-723`) and `GetAlternativeTargetLocation` (`:595-640`) only test points where |dy| = i, never where |dx| = i. So they can wrongly report the target as surrounded, or reject a destination that has free space beside it.
- **The path cache is never cleared and stores failures.**
  - `PathCacheInit` has no callers; the DLL's clear routine resets the DLL's own copy of the cache, not ours.
  - Failed searches are pushed too (`:1669`).
  - A cached path to a target is keyed by the target object, not its position (`:292`).
  - Entries expire only after 5 s.
- **Clearance index out of bounds.**
  - Sectors with no `.sec` file are stored as `-1` in a `uint16_t` (65535) and used without a check (`:1336`, `:2321`).
  - `clrAddr[16][16]` limits clearance data to 1024 × 1024 tiles, so a larger module map reads past the array.
- **The clearance search ignores blocking scenery.**
  - Clearance data only holds sector tile flags.
  - `OLC_PATH_BLOCKER` (`objlist.h:31`) = creatures + doors + containers, with no scenery.
- **`ProximityList` overflow.** Past 256 objects within 40 tiles it only logs and asserts, then writes past the stack buffer (`:33-57`). The avoid-attacks-of-opportunity branch leaks a Python tuple per object and doesn't check for a NULL result (`:108-114`).
- **Diagonal steps are looser than straight ones.** The code requires 0.7 × the clearance on diagonals, despite the comment saying diagonals should be stricter (`:2199`, `:2335`). Nothing checks the two cells beside a diagonal step, so paths cut corners.
- **Smoothing and the search use different collision models.** Smoothing raycasts with 0.7 × the radius (`:1749`) while the search uses the clearance map. A waypoint is added only when the direction changed (`:1108`). The result is cut corners and zig-zags.
- **Search-time budget.**
  - `PathSumTime` sums `aStarTimeElapsed[i]` where it should use `[astarIdx]` (`:343`).
  - `AStarSettingChanged` reads `"a_star_max_time_ms"` where the setting is `"astar_max_time_ms"` (`:371`). The lookup returns 0, so every time-capped search fails after a settings change.

## Performance

- **The open set is a linked list scanned linearly every step**, which makes the search O(n²). A binary heap would fix it.
- **Each search clears a 400 KB grid on the stack** (160² × 16 bytes). A path through nodes runs several searches.
- **The heuristic is 10·max(dx,dy) while diagonal steps cost 14.** That underestimates; octile (`10·max + 4·min`) is better. There is no tie-breaking.
- **An unreachable destination subtile exhausts the whole grid before failing.** The search halts only on the exact cell (`:2283`), and the destination check is a raycast, not the clearance map.
- **The free-spot and surrounded checks** (hundreds of raycasts) run before the cache lookup (`:1577-1598`).
- **Without `clearance.bin`** (most module maps), every search step is a raycast.
- **Outside combat every search writes several `logger->info` lines** (`|| !combatSys.isCombatActive()`).

## Path nodes

- **The nearest node is chosen by straight-line distance** with no reachability check (`path_node.cpp:851`).
- **Nodes are a linked list**, so each lookup is O(N).
- **Without per-link distance data**, `FindPathBetweenNodes` scores nodes by adding up straight-line distances from the start node (`path_node.cpp:954`), which gives poor routes.

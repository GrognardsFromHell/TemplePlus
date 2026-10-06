# Battle Grid (design proposal, not implemented)

A 5-ft square layer used for positioning and movement in turn-based combat, so the game behaves like the D&D battle mat: 8 Medium creatures fit around one Medium creature, and 12 fit around one Large. Motivation and the current pathfinder's problems are in `pathfinding.md`.

Status: design agreed 2026-10-06. Nothing is built yet.

## Scope

- **Only active during turn-based combat**, behind a config flag (`combatGrid`). Movement outside combat stays continuous, and vanilla/Co8 behaviour is the default.
- **Real-time movement and performance are out of scope.** The goal is correct positioning for the AI and for players.
- When grid mode is on, these all use squares:
  - where creatures stand and the paths they take
  - reach and threatened squares (attacks of opportunity)
  - flanking

## Terms

| Term | Meaning |
|---|---|
| **square** | 5 ft = 60 in cell of the lattice |
| **anchor** | top-left square of a creature's footprint (integer square coordinates) |
| **footprint** | N × N squares from the anchor. Small/Medium N = 1, Large 2, Huge 3. Tiny and smaller count as 1 for now; shared squares come later. |
| **origin** | absolute world inches of the corner of square (0, 0) |

Code names: `BattleGridSystem`, singleton `battleGrid`; on the Python side, `game.battle_grid` / `tpdp` helpers.

## Geometry

- **The lattice is fixed to absolute world inches** (see `coordinates.md`), with its axes along the tile axes. On screen each square is a diamond about 85 × 59 px. It does not line up with tiles (28.284 in) or sectors (64 tiles = 30.17 squares).
- **The object's `location` stays at the center of its footprint**, so rendering, line of sight, area effects and the selection circle don't change:
  - `center = origin + (anchor + N/2) · 60`
  - `anchor = round((center − origin) / 60 − N/2)`
  - Odd N stands on a square center, even N (Large) on an intersection. No second grid and no special case are needed.
- **At combat start** each creature snaps to the nearest free anchor: an instant nudge of at most about 42 in. A walk animation can come later.

## Movement and rules

- **Passability:** each square has a precomputed **clearance size** = the largest free N × N block whose top-left is that square (the "true clearance" technique).
  - Anchor A is passable for size N when `clearance[A] ≥ N` and no other creature occupies any square of that footprint.
  - **Squeezing:** allowed when `clearance ≥ ceil(N/2)`, at double cost. This also covers vanilla corridors narrower than 5 ft.
- **Pathfinding:** an 8-direction search over anchors, with the D&D diagonal cost of 5/10/5. A 5-ft step is exactly one square.
- **Melee approach:** the goal is any free anchor from which the target's footprint is within reach. The AI picks the cheapest one and may prefer anchors that leave room for allies or set up a flank.
- **Reach:** the Chebyshev gap between the two footprint rectangles, using the 5/10 diagonal rule for 10-ft reach.
- **Flanking:** the attackers' squares are on opposite sides of the target's footprint.
- **Code these replace in grid mode:**
  - `FindPathShortDistanceAdjRadius`
  - `GetAlternativeTargetLocation`
  - `TargetSurrounded`
  - the reach checks in `combat.cpp` (`CanMeleeTarget*`)
  - `IsFlankedBy` and `AiSystem::Flank`

## Storage: Tiled map JSON (`maps/<MapName>/battlegrid.tmj`)

Formats compared:

| Format | Per-square data | Isometric | Verdict |
|---|---|---|---|
| **Tiled** `.tmj`/`.tmx` | tile layers | yes | **chosen** |
| LDtk `.ldtk` | IntGrid layers (ideal) | no (as far as we know) | the designer would paint a rotated, squashed view |
| Universal VTT `.dd2vtt`/`.uvtt` | none (wall/door segments) | no | worth an **importer** later (Dungeondraft, Dungeon Alchemist) to pre-fill blocked squares and portals |
| Foundry VTT scene | none (walls, grid offset) | no | no |
| ESRI ASCII / GeoTIFF | yes | no | too heavy |
| custom JSON + PNG | yes | no (grid space only) | superseded by Tiled |

Why the `.sec` files don't fit: squares don't line up with sector borders.

Tiled layout:

- **Orientation and scale:** an isometric map with tile size **120 × 84**. That is the engine's 0.7 height ratio, with axes matching the tile axes. 120 px is only the editor's display scale.
- **Map properties:**
  - `originX` and `originY` in world inches. Use these to shift the lattice so squares line up with room walls.
  - `version`.
- **Tile layer `classes`:** CSV encoding, so it diffs cleanly. It uses a small tileset where each tile is a class with custom properties: `blocked`, `cost`, `cover`, `squeeze`.
  - **GID 0 (empty) means "auto"**: work it out from the map.
- **Object layers:** zones, encounter start points and named areas, as rectangles, polygons or points with custom properties that Python scripts can read.
- **Image layer:** a rendered preview of the ground art for editing. The game ignores it.
- **Engine parsing:** the existing `infrastructure/json11.hpp`. No PNG decoder is needed.

**Not stored: passability and clearance.** These are computed at map load from three sources:
- the `.sec` subtile blocking flags
- static blocking objects
- the overrides painted in `classes`

That is about 220k squares and takes a few milliseconds, and it avoids the stale-file problem of today's `clearance.bin`. A map without `battlegrid.tmj` uses origin (0, 0) and computes everything automatically, so vanilla and Co8 maps work without authoring.

## Planned steps

1. Square/anchor maths, computing passability, occupancy, and a debug overlay that draws squares, footprints and anchors. No rule changes yet. Check 8 Medium creatures around 1, and 12 around a Large one.
2. Square-based search and melee approach in `MoveSequenceParse` and the AI; snapping at combat start.
3. Reach, threatened squares and flanking on squares.
4. Tooling:
   - `battlegrid.tmj` loader
   - a `libtemple` script that generates the ground-art preview image layer
   - a sample map for `zmod_kots`
   - later, a Universal VTT importer

# Doors and secret doors

Room architecture is painted into the ground art (`map-files.md`), but **doors are real 3D objects**: `obj_t_portal` with a `.skm` model and `.ska` animations. They're placed in a sector's static object list or as a `.mob`. A **secret door** is a door, or sometimes a scenery object, whose model is textured to match the painted wall it sits in. The sliding effect is the model's own open animation.

Sources: `TemplePlus/secret_door.cpp`, `TemplePlus/gamesystems/mapobjrender.cpp`, `TemplePlus/obj.cpp`, `TemplePlus/temple_enums.h`.

## Flags

`obj_f_secretdoor_flags` (field 47) holds:

| Bits | Meaning |
|---|---|
| `OSDF_DC_0..6` (0x1–0x40) | 7-bit search DC |
| `OSDF_RANK_0..6` (0x80–0x2000) | the searcher's Search ranks at the last failed passive check. Stored so the same character doesn't re-roll every time. |
| `OSDF_SECRET_DOOR` (0x10000) | this object is a secret door |
| `OSDF_SECRET_DOOR_FOUND` (0x20000) | it has been revealed |

There are also `obj_f_secretdoor_dc` (49) and `obj_f_secretdoor_effectname` (48).

`protos.tab` contains no "secret" entries, so the flags are probably set per placed instance (in `.sec`/`.mob`) rather than in the proto. Not verified.

## Before it's found

- **Untargetable** (`Objects::IsUntargetable` → `IsUndetectedSecretDoor`): it can't be clicked, opened or hovered.
- **A portal stays drawn**, which is why its texture has to blend with the wall.
- **A non-portal secret object** (scenery) isn't drawn at all until found (`MapObjectRenderer`, the three "Dont draw secret doors" checks).
- **Movement:** closed portals are path-blocker objects (`pathfinding.cpp`, `OLC_PATH_BLOCKER`, `PQF_DOORS_ARE_BLOCKING`). How pathfinding stops creatures pathing through an undetected door wasn't verified.

## Being found

- **Passively**, when a PC with line of sight comes near (`SearchEventSecretDoorDetect`). It only happens if the PC's Search ranks reach the door's auto-detect DC, then it rolls against the DC.
- **Actively**, with the Search action (`SecretDoorSkillCheck`). That's take-20 plus +2 cooperation bonuses from nearby party members who pass a DC 10 Search check.

## On reveal (`SecretDoorDetect`)

1. Sets FOUND.
2. If `effectname` is set, fades out the nearby object whose `obj_f_name` matches it. That's typically a cover or overlay object.
3. Spawns particles at every bone named `effect-secretdoor-00` … `-28` in the door model.
4. A non-portal object fades in (transparency 0 → 255).
5. Floats `skill_ui.mes` line 1000 over the door.

From then on it's an ordinary door: opening it plays the model's open animation, and an open portal no longer blocks.

Python: `obj.secretdoor_detect(seeker)` (`python_object.cpp`).

## Building one for a new map

This is inferred from the above, not tested.

- Paint the ground art with the **passage open**: the background behind the door.
- Make a door `.skm` textured like the surrounding wall, with an open animation, and optionally `effect-secretdoor-NN` bones for particles.
- Place it as an `obj_t_portal` with `OSDF_SECRET_DOOR` and a DC set in `obj_f_secretdoor_flags`.
- Leave the doorway subtiles **unblocked** in `.sec`, and keep the clipping meshes out of the opening. The door object does the blocking.

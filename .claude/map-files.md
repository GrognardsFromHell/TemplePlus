# Map files and how walls and 3D scenery work

Investigated on `D:\Games\Temple of Elemental Evil\modules\ToEE\maps\TEMPLE_09_Level_1`, against the TemplePlus loaders. Coordinates are explained in `coordinates.md`.

## Walls are not rendered as 3D models

ToEE is a **pre-rendered background game**. Walls, floors, pillars and most room architecture are painted into the **ground art JPEGs**. The 3D wall geometry that exists is **invisible**: it's used only for depth, so characters disappear behind walls correctly.

| Layer | Files | Visible? | Purpose |
|---|---|---|---|
| Ground art | `art/ground/<map>/{y:04x}{x:04x}.jpg` (256 px tiles; `groundArtId` in `map.prp` selects the folder) | yes | the whole painted scene, walls included |
| Clipping meshes | `clipping.cgf` + `clipping.cif` → `art/clip/<map>/*.dag` | **no** (depth only) | invisible 3D shapes of the walls and pillars painted in the background, so characters behind them get hidden |
| GMesh | `gmesh.gmf` + `gmesh.gmi` → `.skm` + `.ska` | yes, lit and animated | real 3D ground meshes, e.g. `art/meshes/scenery/water/floor_clipping` |
| Scenery and portal objects | objects stored in `.sec` (static) or `.mob` | yes | doors, statues, furniture and so on, as `obj_t_scenery`/`obj_t_portal` with `.skm` models |
| Blocking | `.sec` subtile flags (`BlockXiYj`, `FlyOverXiYj`) | no | what can be walked on. **The clipping meshes don't block movement.** |

### Render order (`GameRenderer::RenderWorld`)

1. **Terrain**: the ground JPEGs.
2. Fog checks.
3. **Clipping** (`ClippingSystem::Render`): wall meshes drawn with colour writes off and depth writes on, so they fill only the depth buffer.
4. **Map objects**: critters, scenery, doors and items as 3D models, depth-tested against the clipping depth. Parts behind a "wall" are hidden.
5. **GMesh**, lightning, particles, fog of war.
6. **Occluded objects** (`RenderOccludedMapObjects`): the hidden parts are drawn again as silhouettes with `art/meshes/occlusion.mdf`, so party members stay visible behind walls.

So a new map with 3D-looking walls needs four things:
- the walls painted into the ground art
- matching clipping meshes positioned over them
- blocking flags in `.sec`
- optionally real `.skm` scenery objects for things that must change (doors) or be lit or animated

## Why it was built this way

The level editor (Troika's WorldEd, which shipped with the Polish retail release) shows walls as wireframe architecture, even though the game only shows paintings of them. This suggests the levels were modelled as full 3D scenes, probably in 3ds Max given the era and Troika's pipeline (not confirmed). The game then shipped three things derived from each scene, not the scene itself:

1. **The look**: the scene rendered offline from the fixed isometric camera into the ground art JPEGs. This is a pre-rendered background, like *Baldur's Gate* or *Fallout*. It was far cheaper than real-time 3D on 2003 hardware.
2. **Being hidden behind walls**: simplified copies of the walls and pillars, saved as invisible clipping meshes (`.dag`).
3. **Walls blocking movement**: per-subtile blocking flags written into `.sec`. A 2D grid is far cheaper to search than 3D geometry.

These three only line up because they came from the same source scene. A new map has to keep them in sync by hand or with a generator.

### Terms

| Term | Meaning here |
|---|---|
| **Mesh** | 3D shape made of **vertices** (points), **edges** (lines between them) and **faces/polygons** (usually triangles). `.dag` and `.skm` are meshes. |
| **Wireframe** | Drawing only a mesh's edges. That's what the editor and the "Debug Clipping" view show. |
| **Pre-rendered background** | A 3D scene rendered offline to a 2D image. That image is the ground art. |
| **Orthographic camera** | A camera without perspective, so parallel lines stay parallel. ToEE's camera is orthographic, tilted about 44.4° (`coordinates.md`). It has to be, for 3D characters to line up with the painted image. |
| **Depth buffer (z-buffer)** | A per-pixel "distance from camera" image. When a pixel would be drawn farther away than what's already stored there, it's discarded. |
| **Occluder / clipping mesh** | Geometry drawn only into the depth buffer, never to the screen, so it hides whatever is behind it. That's ToEE's `.dag` clip meshes. |
| **Proxy / low-poly mesh** | A simplified stand-in for detailed geometry. The clip meshes are proxies of the painted walls. |
| **Collision / navigation data** | What movement uses, kept separate from what you see. In ToEE that's the `.sec` subtile flags, not any mesh. |
| **Bounding sphere** | The smallest sphere around a mesh, used to quickly skip off-screen meshes (culling). |
| **Bone / skeleton** | Named points that animate a mesh (`.ska`). Also used as attachment points, e.g. `effect-secretdoor-NN`. |

Doors, including secret doors, are the exception to "walls are painted": they're real 3D objects. See `secret-doors.md`.

## File formats

### `clipping.cgf`: list of clipping mesh files

```
int32 count
count × char[260] path    e.g. "art\clip\map12-temple-dungeon-level-1\map12-temple-dungeon-level-1_archwall90-clip.dag"
```

### `clipping.cif`: clipping mesh instances

Each record is 32 bytes. The sample map has 741 instances.

```
int32 count
count × {
  uint32 meshIndex           // into clipping.cgf
  float  posX, posY, posZ    // world inches, Y = up (same space as LocAndOffsets::ToInches3D)
  float  scaleX, scaleY, scaleZ
  float  rotation            // radians, about the vertical axis
}
```

Example from the sample: mesh 0 at (11626.25, 0, 16906.92), scale 1, rotation 0. That's about tile (411, 597).

### `.dag`: clipping mesh geometry (`clippingmesh.cpp`)

TemplePlus only supports one object per file.

```
float3 boundingSphereOrigin; float boundingSphereRadius
int32  objectCount (must be 1); int32 dataStart
@dataStart: int32 vertexCount, triCount, vertexDataOffset, indexDataOffset
vertices: vertexCount × float3 (position only)
indices:  triCount × 3 × uint16   (TemplePlus swaps the 1st and 3rd index of each triangle to get clockwise order)
```

The bounding sphere is used for screen culling. Only positions matter: no normals, UVs or materials.

### `gmesh.gmf` / `gmesh.gmi`: visible static 3D meshes (`gmesh.cpp`)

```
gmf: uint32 count; count × { char[260] skaPath; char[260] skmPath }
gmi: uint32 count; count × { uint32 fileIndex; float3 pos (inches, Y up); float3 scale; float rotation }
```

They are rendered through the normal animated-model pipeline, with map lights. The sample's `gmi` count is 0, so no instances.

### `.sec`: sector

Covers tiles, lights and tile scripts (see `coordinates.md` and `libtemple/map.py`), and ends with an object list that starts with `0x77`. These are **static objects** saved with the sector. In the sample they're mostly `portal` (doors) and `scenery`.

### `.mob`: map mobiles

One file per object, named `G_<GUID>.mob`, version `0x77`. Loaded by `MapSystem::ReadMapMobiles`. **This isn't only NPCs.** In the sample's 1107 files: 226 npc, 313 armor, 257 money, 207 weapon, 40 food, 34 ammo, 18 container, and a few others. They're objects placed in the map that can move or change, as opposed to the static ones in `.sec`.

### `.svb`: sector visibility buffer

- One per sector, with the same name as its `.sec`. Always 18 432 bytes = 192 × 192 subtiles (64 tiles × 3) at 4 bits each.
- Loaded by `SectorVBSystem` (`legacymapsystems.h`; struct `SectorVB` in `gamesystems/map/sector.h`) and used by fog of war / line of sight (`MapFoggingSystem::PerformCheckForCritter`).
- The sample values are mostly 0, then 0x11 and 0x10, so the per-subtile flags are sparse.
- Most likely they mark subtiles that block sight, like walls. That isn't confirmed: the flag meanings live in `temple.dll`, so check `temple.c`.

### Other files

| File | Contents |
|---|---|
| `map.prp` | `int32 groundArtId; int32 unused; uint64 limitX; uint64 limitY`. Sample: art 86, 960 × 960 tiles. |
| `global.lit` | Global map light (type, colour, direction). |
| `fog.col` | Fog colour (8 bytes). |
| `pathnode.pnd` | Long-range pathfinding nodes. TemplePlus prefers `pathnodenew.pnd`, and also uses `pathnodedist.pnd` and `clearance.bin` (`path_node.cpp`). |
| `mapinfo.txt` | e.g. `SoundScheme: 8,0`. |
| `startloc.txt` | Start tile (x, y). TemplePlus notes it's only used by WorldEd, not the game. |

## Tools

- The original level editor, Troika's WorldEd, shipped with the Polish retail release. The `.dag` clip meshes were most likely exported from the same 3D scene that was rendered into the ground art (see "Why it was built this way"). That's an assumption; nothing in this repo shows the pipeline.
- Without WorldEd, a module needs its own generator. For clipping that means simple prism or box meshes per wall segment, written as single-object `.dag` files, plus a `.cif` that places them in world inches.
- The **"Debug Clipping"** checkbox in the in-game debug UI (`ui/ui_debug.cpp`, sets `config.debugClipping`; it isn't an ini setting) draws the clipping meshes as wireframe in-game. It's the quickest way to check that generated clip geometry lines up with the painted walls.

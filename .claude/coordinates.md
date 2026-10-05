# Coordinate systems

ToEE layers D&D's feet on top of Arcanum's tile and sector grid. These are the six units in play, from coarse to fine. Sources: `Infrastructure/include/infrastructure/location.h`, `Infrastructure/src/location.cpp`, `TemplePlus/location.cpp`, `TemplePlus/gamesystems/map/sector.h`, `Infrastructure/src/graphics/camera.cpp`, `TemplePlus/graphics/mapterrain.*`. A Python reader/writer for `.sec` files is in `C:\Users\anato\source\repos\github.com\Anatoliy.Savchak\libtemple\libtemple\map.py`.

| Unit | Size | Where it's used |
|---|---|---|
| **Sector** | 64 × 64 tiles | `.sec` files: tile flags, lights, tile scripts, static objects |
| **Tile** (`locXY`) | 28.284 in ≈ 2.357 ft per side | object `location`, `location_from_axis(x, y)`, map/jump points |
| **Subtile** | 1/3 tile = 9.428 in | blocking and fly-over flags in `.sec` (`BlockX0Y0` … `BlockX2Y2`), pathfinding, clearances |
| **Inch** | world unit | `LocAndOffsets` offsets, object radius and height, 3D positions |
| **Foot** | 12 in | D&D rules: reach, movement, spell ranges, AoE |
| **Screen pixel** | see "Screen projection" below | rendering, ground art, mouse picking |

## Tiles and offsets: `locXY`, `LocAndOffsets`

- **`locXY`** = `{uint32 locx, uint32 locy}`, packed into one 64-bit value with x in the low 32 bits and y in the high 32 bits. This is what `obj.location` returns in Python, and `location_from_axis(x, y)` builds it.
  - Vanilla maps are centred on tile (480, 480). The real size comes from the map's `map.prp`.
- **`LocAndOffsets`** = `locXY` plus `off_x`/`off_y` floats, which are **inches from the tile centre**, not from the tile corner.
  - `Regularize()` keeps the offsets in [−14.142, 14.142): half a tile either way, moving to the neighbouring tile as needed.
- **`LocFull`** adds `off_z`, which is height in inches.
- **Absolute world inches**: `X = locx·28.284 + 14.142 + off_x`, and the same for Y with `locy` and `off_y`. `LocAndOffsets::FromInches` converts back.
  - In 3D (`ToInches3D`) the ground plane is X/Z: the tile's y becomes the Z axis, and Y is up.
- **Why 28.284**: it is √800 = √(20² + 20²), the diagonal of Troika's 20 × 14 isometric half-tile. So one world inch is roughly one screen pixel along a tile axis, before the vertical squash.

## Feet and the D&D "5 ft square"

- Distances in rules code are `inches / 12` (`LocationSys::InchesToFeet`).
  - Object-to-object distance **subtracts both radii** (`DistanceToObj`), so "5 ft reach" means from edge to edge.
  - `obj.radius` is in inches.
- **There is no 5 ft grid.**
  - Movement and positions are continuous in inches. One 5 ft square is 60 in ≈ 2.12 tiles, so D&D squares don't line up with tiles or subtiles.
  - A 5 ft step, a 10 ft reach or a 20 ft burst are all just distances in feet.
- Rule of thumb: **1 tile ≈ 2.36 ft**, **1 subtile ≈ 0.79 ft**, **5 ft ≈ 6.4 subtiles**.

## Sectors and `.sec` files

- A sector covers 64 × 64 tiles: sector index = `tile // 64`.
- `SectorLoc` packs as `(sy << 26) | sx`. The `.sec` filename is that value written in decimal.
  - Example: `KEEP_LV1/402653191.sec` = `0x18000007` → sector (7, 6) → tiles x 448–511, y 384–447.
- File layout (see `map.py`):
  1. Lights.
  2. 64 × 64 tile records in row order (y outer, x inner), 16 bytes each:
     - 1 byte material, 3 bytes padding
     - 4 bytes flags
     - 8 bytes unused
  3. Roof-list flags, then a version/placeholder value (`0xAA0000`–`0xAA0003`) that controls which optional fields follow: tile scripts, sector script, townmap info, aptitude, light scheme, sound list.
  4. Optional object data, starting with `0x77`.
- Tile flags `0x1`–`0x100` are unused Arcanum leftovers. ToEE uses the **per-subtile** bits:
  - `BlockXiYj` (0x200–0x20000) and `FlyOverXiYj` (0x40000–0x4000000), where i and j are the subtile column and row, 0–2.
  - `ProvidesCover` (0x8000000) applies to the whole tile.

## Screen projection (isometric)

- Tile corner to screen pixels (`LocationSys::TileToScreen`):
  ```
  sx = (y - x - 1) * 20
  sy = (x + y) * 14
  ```
  - A tile is drawn as a 40 × 28 px diamond.
  - **+x tile axis** points to the screen's lower-left.
  - **+y tile axis** points to the screen's lower-right.
  - Scrolling adds `translationX/Y` (`ToTranslation`).
- The 3D camera (`WorldCamera::Update`) uses the same geometry:
  - orthographic projection
  - a rotation about X of −0.77539754 rad (44.427°, where sin = 0.7 = 14/20), so tile height on screen is 70 % of width
  - screen Y scaled by 20/14 when computing the camera translation
- So a **top-down 2D map rotated 45° and squashed vertically to 0.7** matches ToEE exactly. Drawn at **1 px = 1 inch** (28.284 px per tile, 60 px per 5 ft), the 45° rotation makes each tile 40 px across, and the 0.7 squash makes it 28 px tall, which is the engine's diamond.
  - Use the formula above to check which way the axes go.
  - Rotating 45° keeps distances. So, before the squash, screen pixels along a tile axis are in the same units as world inches.

## Ground art (terrain JPEGs)

- 256 × 256 px JPEG tiles named `{y:04x}{x:04x}.jpg` (`TerrainSystem::GetTileTexture`), at most 66 × 71 tiles (16 896 × 18 176 px).
- The grid is centred on the map's centre tile, with an unexplained (−40, −14) px shift that may be an Arcanum leftover (`TerrainSystem::Render`).
- So for a generated background:
  1. Render the isometric image in screen space.
  2. Place it so the map-centre tile lands at the image centre plus (−40, −14) px.
  3. Cut it into 256 px tiles.
  4. Write the matching `.sec` subtile blocking flags using the tile/subtile formulas above.

## Common mistakes

- Treating `off_x/off_y` as offsets from the tile corner. They are from the **centre**.
- Swapping x and y: they're swapped in `ToInches3D` (tile y → world Z) and on screen (+x goes left).
- Treating feet as a grid. Rule ranges are continuous distances, edge to edge minus radii.
- Forgetting that `locXY` is unsigned. Negative offsets must be regularized into the neighbouring tile, not stored as negative tile coordinates.

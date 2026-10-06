# HydroCoupleComposer hand-off — interface ABI 4

**Date:** 2026-09-30
**Follows:** `plans/sdk/ABI4_HANDOFF_2026-09-29.md` (SDK port, verified green on the Mac in `2b960aa`); the Composer was flagged there as C1.
**State:** seven files ported; the test stubs were **compile-checked here** against the ABI-4 headers and the ported SDK (`g++ -std=c++20 -fsyntax-only`, with a two-function GDAL shim for `OGRSpatialReference`). The production sources need Qt and GDAL and were **not compiled** — the Mac build is the verification. Uncommitted in the Composer working tree.

## Changes

| File | Change |
|---|---|
| `src/layers/dataitemlayer.cpp` | `entityAxis()`: network and surface items answer `entityDimension()` directly (the three-accessor switch is gone). `loadGeometry()`: `location() == MeshLocation::Edge` / `Node` replaces `networkDataObjectType()` / `meshDataObjectType()` with `Vertex`/`Cell`. Behaviour identical: the values are still drawn on the entity they are attached to. |
| `src/layers/meshlayer.cpp` | `meshFromGrid()`: node counts and loop indexes are `int64_t` (the grid interface widened). |
| `src/layers/rasterdataitemlayer.cpp` | Explicit `int` casts from the now-`int64_t` `xSize()/ySize()` into the layer's `int` width/height. |
| `include/gis/spatialreference.h`, `src/gis/spatialreference.cpp` | Implements the new vertical CRS accessors. For a **compound** CRS GDAL's `VERT_CS` node supplies authority, code, WKT and linear units; a horizontal-only CRS answers empty / 0 / `Unknown` — "no vertical datum declared". `cacheIdentity()` fills the new members once. |
| `tests/gui/spatialstubs.h` | `StubCrs` vertical accessors (undeclared). `StubTimeSurfaceItem` / `StubTimeGeometryItem`: `location()`, `entityDimension()`, `vectorBasis()`, and the four temporal-semantics accessors (Instantaneous / `None` / `Refuse` — the stubs serve exactly the instants they hold). `StubGrid`, `StubGridItem`, `StubRaster`, `StubRasterBand`: `int64_t` widths, `location()`, typed `read()/write()` over `BufferDescriptor`, `geoTransformation()` const. `makePoint/makePolygon` take `int64_t` indexes. |
| `tests/gui/test_dataitemlayers.cpp` | `MeshDataObjectType::{Vertex,Edge,Cell}` → `MeshLocation::{Node,Edge,Face}` in the parametrised cases. |

## Verification checklist (Mac)

1. Build the Composer against the ABI-4 HydroCouple/SDK trees (`98bfbaa` / `2b960aa`). Expect no warnings from these files beyond what the Qt build already emits.
2. `tests/gui/test_dataitemlayers` — the three surface cases (node/edge/face) exercise exactly the code that changed; `test_difference` unchanged.
3. Grep the Composer for `MeshDataObjectType|patchDimension|vertexDimension()|edgeDimension()` — zero hits expected (the `m_edgeDimension`/`cellEdgeDimension` grid members are unrelated and stay).
4. Optional: a compound-CRS fixture (EPSG:5498, NAD83 + NAVD88) through `SpatialReference::fromAuthority` to see `verticalAuthSRID() == 5703`.

## Not done

- The Composer does not yet *use* the vertical datum (no comparison when two layers are overlaid). That is the validator work item 6 of the review left to the SDK/Composer; the data is now available.
- `hydrocouplecomponentabi.h` (the peer's uncommitted HydroCouple addition) was not looked at; the Composer's own `include/plugins/componentabi.h` was untouched.

## Mac result (2026-09-30)

Verified; **not committed** (the hand-off did not ask for it, and the
Composer holds a stale, empty `.git/index.lock` from 2026-09-29 21:44).

1. Build: 424/424, 0 errors, no new warnings from the seven files.
2. `test_dataitemlayers` 31/31 after one fixture fix; `test_difference` 13/13.
3. grep: 0 hits.
4. EPSG:5498: `EPSG`/5703/Meters; EPSG:4326: undeclared.

Full suite: 64/65. `test_viewhandoff` fails 3 tests identically on the
pre-port Composer against ABI 2 (A/B), so it predates this round.

Fix: `AnEdgeItemOnASurfaceWithNoEdgesIsRefused` also clears the faces.
Since SDK `2b960aa` the mesh view derives edges from face loops, so
clearing only `edgeNodes` left five edges. The gate was seen to fail with
the refusal disabled.

Environment: both installs were ABI 2 and were reinstalled from `98bfbaa`
/ `2b960aa`. The SDK install must come from a static-triplet tree; the
`macos-full` dynamic tree's install is not self-contained. The old
`build/darwin` pins the removed macOS 26.5 SDK, so a fresh
`build/darwin-abi4` was used.

Record: `HydroCoupleComposer/verification/MAC_VERIFICATION_ABI4_2026-09-30.md`.

**Commit (2026-10-01):** `919c8f4` "feat: port to interface ABI 4" (the seven
files plus the fixture fix), held on branch **`v2-abi4`**, not on `v2`.
The Composer has a stale `.git/HEAD.lock` (empty, 2026-09-21 09:46, left by
the `fbd9384` commit) and a stale `.git/index.lock` (empty, 2026-10-01
03:56). Moving the checked-out `v2` needs `HEAD.lock`, and locks are not
removed without Caleb's say-so. To land it once he has cleared both:
`git merge --ff-only v2-abi4 && git branch -d v2-abi4`, then `git status`
should be clean for these seven files.

# HydroCoupleComposer — UX Coherence Review and Plan (Phase U)

**Status:** PROPOSED — for review. Nothing here has been executed; the
modernization plan is untouched until this is approved. · **Date:** 2026-09-19
· **Author:** Caleb Buahin (drafted with Claude) · **Reviews:**
`plans/COMPOSER_MODERNIZATION_PLAN_2026-08-24.md` at `fb24baa`, against the
working tree of HydroCoupleComposer, HydroCoupleSDK, HydroCouple, FVQual and
openswmm.gui as of today.

---

## 0. What was asked, and what this document does

Seven asks, each taken to the source rather than to the plan's description of it:

1. The welcome screen should be closeable, as in openswmm.gui.
2. Arguments should have complex, context-specific editors by argument type, as standalone dialogs.
3. Anything opened in the 2D view should also render in the 3D view, in its 3D form where it has one.
4. The 3D view should carry the classic three-cone axis gizmo at the bottom-left showing cardinal directions.
5. 2D and 3D should have separate controls and zoom options where that is warranted.
6. Selection should propagate across views.
7. Global preferences should be configurable, as in openswmm.gui.

§1 is the review: where the existing plan is stale, internally inconsistent, or
silent on these asks, with file-and-line evidence. §2 states the design rules
the new work has to honour. §3 is the work plan — Phase U, seven items with
slices, decisions, and the gate that closes each — in the same shape as phases
A–F. §4 lists the decisions that are yours to make before U starts. §5 is the
proposed edit list for the modernization plan itself.

---

## 1. Review of the existing plan

### 1.1 Status against the seven asks

| Ask | Model layer | UI today | Plan coverage |
|---|---|---|---|
| 1 Closeable welcome | `WelcomePage` is a permanent first tab of `m_workspace`; "show on start up" is persisted (`recentcompositions.cpp`) | ❌ no close button, no way to re-open other than restart | `fb24baa` entry only; not planned as closeable |
| 2 Typed argument editors, standalone dialogs | `ArgumentDescriptor` chooses 8 kinds from `DataKind`/rank/`IQuality`/`fileFilters` (`argumentdescriptor.cpp:200-235`); all editors are inline widgets in the *Arguments* dock (`componentconfigurator.cpp:220-570`) | ⚠️ entry only; a rank-2 table is a bare `QTableWidget`; a time series is indistinguishable from a table | **B5a** names the editors but keeps them inline; "full B5a typed-editor sweep stays here as future scope" |
| 3 2D ⟷ 3D parity | One `LayerStackModel` feeds `MapCanvas` and `SceneView` (`composermainwindow.cpp:295-340`); every layer class implements `ISceneSource` | ⚠️ points contribute nothing in 3D (`featurelayer.h:196`); a component's polyhedral-surface item draws as draped rings, never as a surface (`DataItemLayer : FeatureLayer`) | C3c-1 records the point and polygon limits as deliberate; nothing schedules closing them |
| 4 Axis gizmo | `Camera` has a north-anchored azimuth (`camera.h:90`) | ❌ no gizmo, compass, or axes anywhere in `scene/` | Not planned |
| 5 Separate 2D/3D controls | `SceneView` has its own tools (`Orbit/Select/ZoomIn/ZoomOut`), projection, exaggeration | ⚠️ Zoom In / Zoom Out / Full Extent are one action each, dispatching on the current tab (`composermainwindow.cpp:383-415`); the ribbon *3D* tab has no Navigate group; no peel slider although `MeshLayer::setVisibleLayers` exists and was optimised for it (C3b-3); tab-switch camera hand-off is unconditional | C5d covers projection + exaggeration only |
| 6 Selection propagation | One selection per `FeatureLayer`, `LayerStackModel::selectOnly` enforces one layer at a time; map, 3D and attribute table are three views of it (C4a–c); plot/profile follow it (D3) | ⚠️ the composition canvas, the layer tree, the run browser and the configurator are **outside** it: selecting a component does not highlight its layers, and vice versa | C4 scoped to the map family only |
| 7 Global preferences | `QSettings` holds exactly three things: `appearance/mode`, recent list, show-welcome; every tolerance is a `constexpr` (`kClickSlopPixels` in three files, `kSnapPixels`) | ❌ no dialog, no manager | **A2 explicitly deferred** the registry search-path UI "to the preferences dialog, which does not exist"; nothing since has created it |

### 1.2 Staleness and internal inconsistencies in the plan text

These are worth fixing whether or not Phase U is approved, because a plan
that disagrees with itself stops being the thing people check against.

- **Decision numbers collide.** D15/D16/D17 are assigned twice (C1b: QPainter map, thin base, lazy viewport; C3a: QRhi, widget-free renderer, per-vertex colour). D26 twice (C1d extent maths; C5d drape hoisting). D27/D28 twice (C2 grid-to-mesh, WKB; D1 catalog timing, entry identity). Anything citing "D16" today is ambiguous. Proposed: keep numbers unique, renumber the later duplicates (C3a → D30–D32, C5d → D33, D1 → D34–D35) and add a decision index at the end of §3.
- **G5 and D5 still say "QSG" for 3D** ("3D-native QSG rendering", "3D via custom QSG/QRhi geometry"); C3a decided QRhi and rejected QSG explicitly. The goal and the decision should read what was built.
- **§1.2 says FVQual's HydroCouple component "is not yet implemented" and E6 gates on it.** `FVQual/include/fvqual/component/fvqualcomponent.h` exists and declares `PolyhedralSurfaceArgument`, `TimeSeriesArgumentDouble`, `Argument1DInt/Double/String` arguments. E6 is at least partly satisfied and should say so; the CONNECT note ("E5/E6 unblock via P3") already hints at it.
- **The status header is behind the body.** It stops at C5/D1-ish; the body records D2–D4, E1, E2a complete. M2 and M3 are reached by the milestone table's own definition (C1–C4, D1–D4 done) but only M1 is ticked.
- **C1's description is corrupted.** At the end of the C1d bullet the original C1 text ("Bring over openswmm.gui's map canvas + QSG renderer + isublayer contract …") is spliced onto the last sentence of a falsification note. Two paragraphs need separating.
- **§8 "Open questions (answer before A1 starts)"** are all resolved by the body (Q1 repo strategy implicit in the tree; Q2 answered at A2; Q3 Qt Charts used in D3; Q4 GDAL hard; Q5 Darwin only, presets unexercised). Close them with one-line answers.
- **B5a's introspection rule needs a caveat the SDK forces.** B2 correctly decided kind comes from what the argument advertises. But the SDK's `TimeSeriesArgumentDouble` and `PolyhedralSurfaceArgument` derive from `AbstractArgument` only — they do **not** implement `ITimeSeriesComponentDataItem` / `IPolyhedralSurfaceComponentDataItem` — so `dynamic_cast` cannot tell FVQual's meteorology series from a rank-2 table, nor its mesh from anything. This is the plan's own "SDK gap discovered mid-build" row and it blocks ask 2 unless closed upstream (U2-S below).
- **Minor:** `E2b–E2d` are stubs with no verify line; §9's licence caution ("openswmm.gui is GPL-3.0 — reimplement behaviour, never copy source") should be lifted to §3 as a decision since it now governs U1, U7 and any future port.

### 1.3 Coherence observations that are not defects, but shape the work

- **The workspace and the ribbon are already coupled two ways** (`composermainwindow.cpp:1237-1277`): switching the workspace tab selects the *map*/*3D*/*home* ribbon tab, and picking a ribbon tab brings its view forward. Every 3D-specific control added must respect that (the exaggeration spin already brings the 3D tab forward). Closing the Welcome tab must not leave the ribbon pointing at a tab whose view is gone.
- **`QTabWidget::setTabsClosable(true)` is the wrong tool** for ask 1: it puts a close button on Map, 3D and Composition too, and those are not closeable. Per-tab close buttons via `tabBar()->setTabButton(index, QTabBar::RightSide, …)` is the mechanism.
- **The renderer is widget-free by force** (C3a: `QRhiWidget` cannot create a device under the offscreen platform). The gizmo therefore has to be drawn by `SceneRenderer` through RHI, not painted over the widget — otherwise every pixel gate for it is untestable in CI.
- **Selection colour is baked per vertex** (C3a) and `FeatureLayer::colorFor` already returns the selection colour for selected features (`featurelayer.cpp:859`), so a selection change triggers a scene rebuild through `appearanceChanged`. That is correct and is why 3D already shows the map's selection; it also means selection style must stay a *layer* concern when it becomes a preference.
- **Undo already covers argument edits** (A4 whole-spec commands; B2 offers a payload to the live component before the document records it). Standalone dialogs must commit through `ComponentConfigurator::applyArgument` and nothing else, or a dialog becomes a second write path with different validation.

---

## 2. Rules for Phase U

- **R1 — One document, several views** (CLAUDE.md §5.1, D8). A dialog, a gizmo, a preference is a view or a control over state that already has an owner. No new owners of layer state, selection or arguments.
- **R2 — Behaviour from openswmm.gui, never source.** openswmm.gui is GPL-3.0; Composer copies token values (D12) and re-implements behaviour clean-room (as `fb24baa` did for the welcome page and `e6c6727` for credentials).
- **R3 — Introspection, not names** (B2/B5a). Editor kind comes from the interfaces an argument implements and the value definition it carries. Where the SDK does not expose the interface, the fix is upstream, and the editor waits.
- **R4 — Every slice ships an offscreen gate and a falsifier** under `verification/u<N>/`, judged on exit status as well as output (C4c's lesson).
- **R5 — Preferences are read at use, applied live.** A preference nobody re-reads is decoration (E4's "no silent CPU-path omissions").

---

## 3. Work plan — Phase U (UX coherence)

Items are ordered so that each one's infrastructure is in place for the next:
U7 (preferences) first because U1, U5 and U6 each need a home for a setting;
U6 (selection hub) before U3 so the 3D representations it adds are selectable
from day one; U2 last because it is the largest and depends on an SDK slice.

### U7. Global preferences. *(first — everything else stores something here)*

- **U7a — `PreferencesManager`.** A singleton `QObject` over `QSettings` in
  `core/`, typed accessors with defaults, one signal
  `preferenceChanged(group, key)`. Groups, seeded from what the code already
  hard-codes plus what the other items need:

  | Group | Keys (initial) | Today |
  |---|---|---|
  | General | show welcome on start-up; reopen last composition; recent list length | `RecentCompositions` holds two of these — they move here and `RecentCompositions` reads through |
  | Appearance | theme mode; ribbon Full/Compact | `appearance/mode` moves here |
  | Components | library search directories; rescan on start-up; accept unstamped libraries (D11) | **A2's deferred UI** |
  | Selection & Picking | click tolerance px; drag threshold px; snap tolerance px; clear selection on miss; selection colour | `kClickSlopPixels` ×3, `kSnapPixels`, `kSelectionColor` |
  | Map | default map CRS (auto from first layer / fixed EPSG); default basemap; scale presets; coordinate readout decimals; default tool | `SpatialReference::webMercator()` hard-coded at `composermainwindow.cpp:321` |
  | 3D View | background colour; default projection; default exaggeration; gizmo visible/size/corner; orbit sensitivity; invert wheel; **link 2D/3D view on tab switch** (U5) | `m_background` literal in `sceneview.h` |
  | Simulation | default writers and manifest folder; thread count; auto-open run after finish | none |
  | Plots | axis number formats; export delimiter | none |

  → verify: `test_preferences.cpp` — defaults when the store is empty; every
  setter round-trips through a scratch `QSettings`; every setter emits exactly
  one `preferenceChanged` with the right group/key; a setter with an unchanged
  value emits nothing.

- **U7b — consumers read live.** Each hard-coded constant above is replaced
  by a read at the point of use (tools read on press, not at construction).
  → verify: for each migrated constant one gate that changes the preference
  and observes the behaviour change *without* recreating the widget — the
  click-slop gate in `test_maptools` becomes parameterised on the preference.
  Falsifier: revert any one read to its constant; the gate must fail.

- **U7c — `PreferencesDialog`.** Category list on the left, `QStackedWidget`
  of scrolled pages on the right, *Reset to defaults* | *Apply* / *Cancel* /
  *OK* (openswmm.gui's shape, re-implemented). `openAtCategory(name)` so the
  Components ribbon button and the 3D tab can open straight to their page.
  Under *File ▸ Preferences…* (macOS: application menu, `QAction::PreferencesRole`)
  and a ribbon *View ▸ Preferences* face.
  → verify: `test_preferencesdialog.cpp` — round-trip: set every widget,
  OK, reopen, every widget shows what was set (openswmm.gui's own gate shape);
  *Apply* changes the manager without closing; *Cancel* after edits changes
  nothing; *Reset* restores defaults and the widgets follow.

### U1. A closeable welcome page.

- **U1a — close in place, re-open on demand.** A close button on the Welcome
  tab only (`QTabBar::setTabButton`). Closing *hides* the tab (`removeTab`
  without deleting the page; the page keeps its recent list) and brings the
  Composition tab forward; the ribbon follows because the existing
  tab-change hook already maps `m_canvas → home`. *Help ▸ Welcome* (and the
  same action on the ribbon *View* tab) re-inserts it at index 0 and selects
  it. Opening or creating a document closes it, as today (`:2599`).
  → verify: extend `test_welcome.cpp` — the close button exists on Welcome
  and on no other tab; after close the current tab is Composition and the
  ribbon is *home*; *Help ▸ Welcome* brings it back at index 0 with its
  recent list intact; closing twice is harmless; the start-up preference
  (now `PreferencesManager`) still decides the first tab.

- **U1b — what the page offers.** Small additions that make closing it a
  loss the user notices: *Examples* section listing the SDK's shipped
  example compositions (`serial_coupling` and siblings, found through the
  SDK's install prefix — no copies), *Load component libraries…* stays, and
  a *Preferences…* link. Clean-room; openswmm.gui's example seeder is a
  behaviour reference only.
  → verify: an example entry opens the composition through the same path as
  *Open* (it appears in *Recent*); a missing SDK example directory yields an
  empty section, not a broken link.

### U6. Selection that crosses views.

The map family is already one selection. What is missing is the mapping
between a **component** (canvas, configurator, execution panel) and the
**layers derived from its outputs** (map, 3D, tree, table, plots), and
between a **run item** (run browser) and its layer.

- **U6a — provenance on layers.** `DataItemLayer` records the component id
  and output id it was created from (`Add Layers from Components` and
  `Show on Map` both know them at creation); `RasterDataItemLayer` and
  `DifferenceLayer` likewise (the difference carries both). Persisted in
  the presentation sidecar so it survives reopen.
  → verify: `test_dataitemlayers` — a layer made from component *c*'s
  output *o* reports `(c, o)`; sidecar round-trip keeps it.

- **U6b — `SelectionHub`.** A `QObject` in `ui/` (not in `map/`: it knows
  about the canvas, which the map must not) that owns *no* selection of its
  own; it subscribes to `CompositionScene::selectionChanged` and
  `LayerStackModel`'s selection signals and forwards each to the other side
  with a re-entrancy guard (C4c's crash is the reason the guard is tested):
  - canvas selects component → the layer tree highlights that component's
    layers and makes the first of them current (attribute table follows by
    the existing C4c rule); the map does **not** change its feature
    selection (a component is not a feature).
  - a feature is picked on the map or 3D → the canvas selects the owning
    component, the configurator shows it (it already follows the canvas).
  - run browser item chosen → its layer, if shown, becomes current in the
    tree.
  → verify: `test_selectionhub.cpp` offscreen with the real window — each
  direction, plus: selecting a component with no layers clears nothing;
  a layer with no provenance (a GDAL file) selects no component; the
  guard: a synthetic ping-pong stops after one hop (falsifier: remove the
  guard → must fail by *exit status*, it recurses).

- **U6c — additive selection.** Shift-click adds, Ctrl/Cmd-click toggles, on
  the map, the 3D view and the attribute table — C4a's "not built, nothing
  asked" item, now asked. The rubber band in 3D (`selectIn`) already returns
  a set. `selectOnly` keeps its one-layer rule; the set within the layer
  grows.
  → verify: extend `test_picking` and `test_attributetable` — shift adds,
  ctrl toggles, plain click replaces; the table's row selection equals the
  layer's set; the 3D view re-bakes the colours (image gate: a second
  feature turns the selection colour).

### U3. What the map shows, the scene shows — in its 3D form.

Shared stack means every layer is already *in* the scene; this item closes
the cases where a layer is there but invisible or flattened.

- **U3a — points in 3D.** Point features are drawn as camera-facing
  markers (screen-sized quads at the draped elevation, the marker's size in
  pixels, in the layer's symbol colour), through a third pipeline beside
  surfaces and ground planes (a sampler-free quad with a per-instance
  centre, size from the uniform block). Off-terrain points sit at the
  layer's own Z when the geometry carries one, else at zero, exactly as
  lines do. The coplanar step count gains a level for markers (surfaces 0,
  ground 1, lines 2, markers 3).
  → verify: `test_scene3d` — a point layer on a terrain produces one marker
  per feature at the sampled height (assert on vertex positions, not
  pixels); markers face the camera after an orbit; picking a marker
  identifies the point (extend `test_picking` scene cases — markers are
  picked by screen distance, since ground-first picking has no ground under
  a station on a cliff).

- **U3b — surface data items are surfaces.** An `IPolyhedralSurfaceComponentDataItem`
  whose vertices carry Z is promoted: `DataItemLayer::create` builds a
  `MeshDefinition` through the SDK's `MeshViewAdapter` and delegates its
  `sceneGeometry()` to the same prism/surface builder `MeshLayer` uses
  (extracted into `scene/meshsurfacebuilder.*` so there is one copy); it
  offers itself as `ITerrainSource` on `MeshLayer`'s rule (declines when Z
  is absent). A layered item (a `{layers, geometries}` shape with a
  vertical coordinate the item exposes) draws as prisms and peels.
  → verify: the C2/D2 surface fixture with Z draws faces in 3D (triangle
  count equals `MeshLayer`'s for the same mesh — the delegation gate);
  without Z it draws rings as today; a draped network lands on it.

- **U3c — per-layer 3D representation, chosen in the dialog.** The
  *Rendering* tab (C5a) gains a *3D* group: Flat / On terrain / Extruded
  already exist; add *Marker size* (points), *Fill polygons* (off by
  default — a filled polygon against terrain is a constrained
  triangulation; offered only when the ring is convex or the layer is flat),
  *As surface* (U3b items), and the layering peel range for prismatic
  layers — **the peel slider finally gets a UI**, here and as a
  ribbon *3D ▸ Layers* group that acts on the current layer.
  → verify: `test_layerproperties` — the tab offers exactly the controls the
  layer supports; a peel set in the dialog and one set on the ribbon are the
  same value (one setter); a filled polygon appears/disappears on toggle.

- **U3d — time in 3D.** D2's verify line asked for "image tests at 3
  timestamps" coherent across 2D and 3D; the D2 results record 2D only.
  Add the 3D half: stepping the clock re-bakes the scene's colours for
  `ITimeLayer`s (via `appearanceChanged`, which already triggers rebuild),
  and the gate compares the class index per feature in both views at three
  instants.

### U4. The axis gizmo.

- **U4a — `AxisGizmo` in `SceneRenderer`.** Three cones on shafts, world
  axes: **E** (+X, red), **N** (+Y, green), **Up** (+Z, blue), with letter
  labels, drawn last into a small square viewport at the bottom-left
  (default 96 px, corner and size from preferences) with its own
  orthographic projection that takes only the camera's *rotation* — so it
  never translates or scales with the scene and is untouched by vertical
  exaggeration (the axes are directions, not distances). Depth cleared for
  the sub-viewport so it always sits on top. Labels as a tiny texture atlas
  baked at start-up. Camera azimuth 0 = north (`camera.h:90`) means N
  points up-screen in the default view, agreeing with the north-up map.
  → verify: `test_scene3d` via `renderSceneToImage` — with azimuth 0 and a
  straight-down elevation, a green pixel column lies above the gizmo origin
  and a red one to its right; after rotating the azimuth by 90°, they swap;
  changing exaggeration ×10 leaves the gizmo image byte-identical; the
  gizmo is absent when the preference hides it (a scene with nothing in it
  renders a blank corner).

- **U4b — click to orient.** Clicking a cone snaps the camera to look along
  that axis (top view for Up; north or east elevation for N/E), preserving
  the target and distance; the classic three.js ViewHelper behaviour.
  In scope for the first gizmo slice (decision 3).
  → verify: a click at the projected cone position sets the expected
  azimuth/elevation; a click beside it is an ordinary scene click.

### U5. Separate 2D and 3D controls.

- **U5a — split the shared zoom actions.** *Zoom In*, *Zoom Out* and *Zoom
  to Full Extent* become per-view actions. The **menu** keeps one entry each
  that dispatches to the front view (so shortcuts keep meaning "zoom what I
  see"); the **ribbon** gets its own *Navigate* group on the *3D* tab
  (Zoom In / Zoom Out / Full Extent / **Reset view** (north-up, default
  tilt) / **Top view** / **Look at selection**). Map's Navigate group is
  unchanged. Rationale: the plan's own reason for one action ("means the
  same thing in each") holds for the menu and not for the ribbon, where the
  3D tab today has *no* way to zoom except the wheel.
  → verify: extend `test_viewhandoff` — the 3D zoom actions never touch the
  map's transform and vice versa (assert the other view's extent is
  unchanged), the menu actions dispatch to the current tab.

- **U5b — linking as a preference.** The tab-switch camera hand-off (C3c-3)
  becomes *3D View ▸ Link views: on tab switch / never*, default *on tab
  switch* (the current behaviour). A *Sync now* face on both Navigate
  groups performs the hand-off on demand in the other direction.
  → verify: with *never*, switching tabs leaves each view's framing alone
  (the C3c-3 gates run under both settings; they were written for one).

- **U5c — 3D-only interaction preferences.** Orbit sensitivity, wheel
  direction, pan modifier (middle-drag vs shift-drag), read live from U7.
  → verify: an orbit drag of N pixels rotates by the preference's ratio.

### U2. Argument editors as typed, standalone dialogs.

This is B5a, redesigned around dialogs, plus the SDK slice B5a could not
have seen. The *Arguments* dock keeps one row per argument, but the row is
now a **summary + Edit…** for every kind except the trivially inline ones
(Boolean, Categorical, short Text, Number/Integer with unit), and *Edit…*
opens a dialog owned by the configurator.

- **U2-S — SDK: typed arguments implement their typed interfaces.**
  `TimeSeriesArgumentDouble` gains `ITimeSeriesComponentDataItem`
  (`timeDimension()`, times); `PolyhedralSurfaceArgument` gains
  `IPolyhedralSurfaceComponentDataItem` over the mesh it already holds
  (through the existing `PolyhedralSurfaceAdapter`); `IdBasedArgument`
  already carries `IIdBasedComponentDataItem`. Additive, defaulted where
  possible. Without this, `describeArgument` cannot tell FVQual's
  meteorology from a table. Its own verification in the SDK (mutations on
  the interface answers).

- **U2a — the descriptor learns the typed kinds.** `ArgumentEditorKind`
  widens: `Quantity` (scalar with `IUnit`), `TimeSeries`, `Mesh`,
  `Geometry`, `Raster`, `IdTable`, `DateTime`, `Duration`, `Crs`,
  `LongText`, `Table`, `Raw`. Chosen by `dynamic_cast` on the argument's
  data-item interfaces first, then value definition, then rank/`DataKind`
  (the existing chain). **Never by id or caption** (B5a's rule stands).
  `DateTime`/`Duration`/`Crs` need a value-definition signal: a quantity
  whose unit's dimensions are pure time (`IUnitDimensions`) is a duration;
  a CRS is a `String` argument whose `validComponentDataItemTypes()` names
  `ISpatialReferenceSystem` (to be confirmed against the SDK — if no such
  signal exists, `Crs` stays out until one does, per R3).
  → verify: `test_configurator` — a fixture component with one argument
  per kind describes to that kind; a caption that *looks* like a date on a
  `String` argument stays `Text` (the negative gate B5a asked for).

- **U2b — `ArgumentEditorDialog` and its factory.** Base `QDialog` with
  *OK / Apply / Cancel*, the argument's caption and description as header,
  a validation strip, and one contract: it reads the payload it was given
  and returns a payload; **commit is the configurator's** —
  `applyArgument()` offers it to the live component, and only then to the
  document (B2's rule, unchanged). Dialogs are **modeless** (`show()`), one
  per argument at a time, so the *Mesh* and *Geometry* dialogs can ask the
  user to pick on the map while open; closing a component's row closes
  its dialogs. `ArgumentEditorFactory::create(descriptor, parent)` maps
  kind → dialog; the *raw JSON/YAML* dialog is the fallback for `Raw` and is
  reachable from every dialog as a *Raw…* button.
  → verify: `test_argumentdialogs.cpp` — every kind's dialog hydrates from
  a payload and `serialize()`s back equal (the B2 hydration contract,
  extended per B5a); *Cancel* after edits leaves the component's
  `serialize()` unchanged; a payload the component refuses shows the
  component's message in the strip and does not reach the document
  (mutation: bypass `applyArgument` → must fail).

- **U2c — the dialogs, in order of leverage for FVQual.**
  1. **Table** — spreadsheet with row/column captions from the argument's
     dimensions, paste from clipboard, CSV import/export, add/remove rows
     where rank allows, a sparkline/plot pane for numeric columns.
  2. **TimeSeries** — the table above with a date-time first column, a
     Qt Charts plot beside it (D3's `dateTimeFromJulianDay` is the one
     conversion), CSV import with column mapping, a *from output* binding
     button (B5b's picker, unchanged).
  3. **Mesh** — three sources on tabs: *From layer* (any `MeshLayer` or
     U3b surface in the stack; the map is the picker), *From file*
     (UGRID through `MeshLayer::fromUGRIDFile`), *From output* (B5b); a
     read-only summary (nodes/edges/faces, extent, CRS) and *Show on map*.
  4. **Geometry** — pick features from a layer on the map, draw with the
     E1b tools, or paste WKT; preview as a temporary layer removed on
     close.
  5. **Raster** — file or `GdalRasterLayer` picker with band choice and a
     thumbnail.
  6. **Quantity** — spin box that converts between a display unit and the
     argument's unit; stored in the argument's (the unit-conversion gate
     B5a specifies: entered and stored deliberately differ).
  7. **DateTime / Duration** — calendar + time, or a duration with unit.
  8. **LongText** — plain-text editor with optional syntax highlighting
     chosen by `fileFilters` (`.rxn` → the MSX-style highlighter E5 wants,
     `.json/.yaml` → JSON/YAML); this is where the kinetics editor lands
     without E5 needing its own plumbing.
  9. **IdTable** — id ↔ value grid for `IIdBasedComponentDataItem`.
  10. **Crs** — C5b's picker, if U2a finds a signal for it.
  Each ships with its hydration contract; `IUIProvider` stays as today and
  its button moves onto the row as *Component's editor…*.

- **U2d — the dock row.** Summary text per kind ("3 × 4 table",
  "1 461 steps, 2015-01-01 → 2018-12-31", "12 480 faces, EPSG:26912"),
  the B5b binding chip unchanged, *Edit…*, and the row's validation state.
  → verify: the summary reflects a payload applied through the dialog
  without a rebuild of the dock (one signal, `argumentChanged`).

### Milestone

**M-U** = U7 + U1 + U6 + U3a–c + U4a–b + U5a–b + U2-S + U2a–b + U2c(1–3).
Everything else in U is a stopping point that can follow. Suite target:
every new slice adds a ctest and a falsifier; ASan stays clean.

---

## 4. Decisions

Taken with the user, 2026-09-19:

1. **Argument dialogs are modeless** — map picking works while a dialog is
   open; one dialog per argument; commits go through `applyArgument` only.
2. **Peel control lives in both places** — a ribbon *3D ▸ Layers* group
   acting on the current layer, and the same control in Layer Properties ▸
   Rendering, over one setter.
3. **The gizmo ships with click-to-orient (U4b) in its first slice.**
4. **Ordering is U7 → U1 → U6 → U3 → U4 → U5 → U2.** U2-S (the SDK
   interface slice) may start in parallel with U7 so U2 is not waiting on
   it when its turn comes.

Still open (defaults apply unless changed):

5. **U5b default** for view linking: *on tab switch* (today's behaviour)
   unless told otherwise.
6. **Additive selection modifiers:** Shift adds / Ctrl-or-Cmd toggles (GIS
   convention) unless told otherwise.
7. **Welcome page examples (U1b):** in M-U unless deferred.

---

## 5. Proposed edits to `COMPOSER_MODERNIZATION_PLAN_2026-08-24.md`

Applied only after this document is approved:

- Header: bring status to the body (D2–D4, E1, E2a complete; M2, M3 reached; Phase U added).
- §1.2 / E6: FVQual component exists; E6 reads "delivered — see `fvqualcomponent.h`; E5 unblocked".
- G5, D5: replace "QSG" with "QRhi" and cite C3a's decision.
- Renumber duplicate decisions (D15–D17 in C3a, D26 in C5d, D27–D28 in D1) and add a decision index.
- Repair the spliced C1 paragraph after C1d.
- Close §8 with one-line answers.
- B5a: point to Phase U2 as its execution; add the SDK prerequisite U2-S.
- A2: "registry settings UI deferred to the preferences dialog" → "lands in U7 (Components page)".
- C4a/C4c: "shift-to-add not built" → U6c.
- C3c-1: "points contribute nothing" and "polygons are rings" → U3a/U3c.
- Add §3 rule R2 (licence) as a numbered decision.
- Add Phase U (this document's §3) as a new section between E and F, and M-U to the milestone table.

# HydroCoupleComposer 2.0 — Modernization Plan

**Status:** IN PROGRESS — Phases **A, B (B1–B4, B6), C (C1–C5), D (D1–D4)** complete; **E1, E2a** complete; **M1, M2, M3 reached**; B5a's typed editors and E2b–E4 open; **Phase U (UX coherence) added 2026-09-19** — see §5 Phase U and `plans/COMPOSER_UX_COHERENCE_PLAN_2026-09-19.md`; **U7 (global preferences) verified and committed 2026-09-19 (`c98b456`)**; **U1, U6 and U3 implemented 2026-09-20, awaiting build verification** (`plans/U1_HANDOFF_2026-09-20.md`, `plans/U6_HANDOFF_2026-09-20.md`, `plans/U3_HANDOFF_2026-09-20.md`); **U4 (axis gizmo) implemented 2026-09-20, awaiting build verification** (`plans/U4_HANDOFF_2026-09-20.md`); **U5 (per-view controls) implemented 2026-09-20** (`plans/U5_HANDOFF_2026-09-20.md`); **U2a (typed editor kinds) implemented 2026-09-20** (`plans/U2A_HANDOFF_2026-09-20.md`); **U2-S implemented in the SDK 2026-09-20** (`0ce3dbf`); **U2b (the argument window) implemented 2026-09-20** (`plans/U2B_HANDOFF_2026-09-20.md`); next U2c · **Date:** 2026-08-24, revised 2026-09-20 · **Author:** Caleb Buahin
**Targets:** HydroCouple interface v2.0.0, HydroCoupleSDK v2.1.0 (Composition Spec v1 + stored runs)
**Template:** openswmm.gui (CMake + vcpkg + Qt6 + QSG sublayer rendering)

---

## 1. Where things stand (survey, 2026-08-24)

### 1.1 HydroCoupleComposer today

| Aspect | Current state |
|---|---|
| Build | qmake (`HydroCoupleComposer.pro`), Qt5-era modules, C++11, v1.4.0 |
| Last commit | 2019-09-30 (`dae8722`); 6 files with uncommitted edits; stale `adaptedoutputfactory` branch |
| Size | ~16.5k LOC across `include/` + `src/` |
| Canvas | `QGraphicsView` node/connection editor (`gnode`, `gconnection`, `gmodelcomponent`, `gexchangeitems`, `gadaptedoutput`) |
| Plugins | `QPluginLoader` Qt plugin mechanism (`componentmanager`) |
| Project format | `.hcp` XML via `QXmlStreamReader` (`hydrocoupleproject`) |
| Argument editing | `argumentdialog` — raw XML text editing with a syntax highlighter + DOM model |
| Results viewing | None in-app; delegated to the separate **HydroCoupleVis** OpenGL app (`../HydroCoupleVis` include path) |
| Other deps | QPropertyModel (`../QPropertyModel`), optional Graphviz, MPI/OpenMP defines |
| Tests | None |

Everything above is written against **HydroCouple v1**, whose interfaces were
Qt-based (QString/QVariant/Qt signals). It does not compile against v2.

### 1.2 What changed underneath it

**HydroCouple v2.0.0** (interface repo, active, CMake + vcpkg + Python bindings):

- Fully **Qt-free**: `std::string`/`std::vector`, no QObject anywhere.
- Eventing is the templated **`ISignal<Args...>` / `ISlot<Args...>`** observer pair
  (`connect`/`disconnect` of `shared_ptr` slots, `blockSignals`).
  `IModelComponent : ISignal<const shared_ptr<IComponentStatusChangeEventArgs>&>`,
  `IComponentDataItem : ISignal<const shared_ptr<IComponentDataItemValueChanged>&>`,
  `IPropertyChanged : ISignal<std::string>`.
- **`IUIProvider`** — components can advertise their own editors/viewers via
  `showEditor(void *opaqueUIPointer)` — the GUI hands in a parent-widget pointer.
- Data plane is **typed hyperslabs**: `shape()` + `DataKind` + `getValuesInto()`/
  `setValuesFrom()` with `BufferDescriptor` (DLPack-style), plus `MemorySpace`
  and `Capability` advertisement.
- Rich spatial model in `hydrocouplespatial.h`: OGC-style geometries, quad-edge
  `IEdge`, `INetwork`, `IPolyhedralSurface`/`ITIN`, `IRaster`/`IRasterBand`,
  `IRegularGrid2D/3D`, `IMeshView`, and matching `*ComponentDataItem`s; WKB
  serialization header; temporal and spatiotemporal item families.

**HydroCoupleSDK v2.1.0** (active, CMake + vcpkg, C++20, no Qt):

- `AbstractModelComponent` lifecycle + `errors()` + `capabilities()`; typed
  argument/exchange-item templates with **JSON argument serialization**.
- **Composition Specification v1** (`IO::ModelInitializer`, `compositionspec.h`):
  JSON/YAML composition documents — components, per-argument payloads,
  connections (incl. adapted-output chains, multi-input roles), workflow
  strategy, output writers, manifest path. JSON Schemas installed at
  `share/hydrocouplesdk/schema/`. Round-trip guaranteed via `IArgument::serialize()`.
  **XML is not carried forward** (decision D1 of the SDK's composition plan).
- **Run manifests + stored runs**: `RunRecorder` writes a manifest cataloguing
  every recorded item (artifact, format, variable, kind, shape, units, mesh
  attachment, time axis). **`ResultsModelComponent::open(manifest, …)`** reopens
  a finished run as an ordinary `IModelComponent` — *no model library needed*.
  `execution: { mode: run | open | resume }` per component; `TimeSliceOutput`
  bridges recorded rank-2 `{time,…}` items to live rank-1 inputs.
- Output writers: CSV, HDF5/UGRID, NetCDF/UGRID, GeoPackage — behind a
  single-writer `IOThread`.
- Workflows: `PullDrivenWorkflow`, `TimeSteppedWorkflow` (feedback-loop
  iteration groups), **cooperative pause/stop at sync points** — made for a GUI.
- **`Tools`** (vcpkg feature `tools`): constrained Delaunay `Triangulator` (cdt),
  `QuadMesher` (quad-dominant), `CurvilinearGrid`, `SigmaGrid`,
  `TerrainSampler` (IDW) — all emitting UGRID-congruent meshes.
- Distributed (MPI transport, partitioned items with halo exchange) and Device
  (Kokkos backend seam) layers.
- **Gap:** the SDK deliberately has **no component registry/loader** —
  `ModelInitializer` takes a caller-supplied `ComponentResolver`. Loading
  shared-library components is the host's job. That host is Composer.

**FVQual v0.1.0-alpha.1** (the model the new tooling must configure):

- Fully **3D** hydrostatic FV: unstructured **tri-quad horizontal mesh** +
  **terrain-following sigma layers** (`mesh/horizontalmesh.h`, `layeredmesh.h`,
  `sigmaspec.h`); CE-QUAL-W2 successor.
- IO: **UGRID NetCDF** reader/writer (`io/ugridreader.h`, `layeredugridwriter.h`).
- Configuration surface: mesh + sigma spec, forcing (`sources`, `structures`,
  `outletgroups`, `transfers` — selective withdrawal, rating curves, multi-port
  blending), MSX-convention **`.rxn` declarative kinetics**, open boundaries.
- **The HydroCouple component now exists** (`include/fvqual/component/
  fvqualcomponent.h`, 2026-09: `PolyhedralSurfaceArgument`,
  `TimeSeriesArgumentDouble`, `Argument1DInt/Double/String` arguments);
  water-quality kinetics is in progress (F6.x commits). C API exists.
  *(Survey text of 2026-08-24 said "not yet implemented"; corrected
  2026-09-19.)*

**openswmm.gui** (the template):

- CMake + **vcpkg manifest** (gdal, gtest, nanoflann, hdf5, openssl) +
  `CMakePresets.json` per-OS presets; Qt6 Widgets/Quick/QuickWidgets/Charts/
  Concurrent/Svg/PrintSupport + **ShaderTools** (qsb-compiled scalar-fill
  material shaders); engine consumed as a prebuilt IMPORTED `find_package`.
- **QSG sublayer rendering architecture**: `render/isublayer.h` +
  `sublayers/` (scalarfill, contourband, meshedge/node, meshbc, feature,
  flow/velocity arrows), classification (graduated/categorical, color ramps),
  legend, labeling; `map/` canvas + QSG renderer + snap engine + undo stack;
  CRS via GDAL; WMTS/raster basemaps; tiled LOD for 1M+-cell meshes.
  **2D only** — there is no 3D camera/scene today.
- `tests/gui` ctest suite running `QT_QPA_PLATFORM=offscreen`.

### 1.3 The gap, in one paragraph

Composer is a 2019 Qt5/qmake app speaking a retired Qt-based v1 interface and a
retired XML composition format, with no results viewer, no GIS, no meshing, and
no tests. The SDK has meanwhile absorbed the composition format, results
access, workflows, writers, and meshing algorithms. What remains genuinely the
GUI's job is: **component loading, the composition canvas, schema-driven
configuration editors, a GIS/3D visualization surface, a model-agnostic results
browser, and meshing/BC tooling veneers** — built on openswmm.gui's proven
scaffolding and rendering architecture.

---

## 2. Goals and non-goals

### Goals

1. **G1 — v2-native Composer**: loads Qt-free v2 components, edits and runs
   Composition Spec v1 documents, with `.hcp` XML one-way import.
2. **G2 — Model-agnostic results viewer**: open any run manifest, browse the
   catalog, theme any recorded variable on its mesh/network/raster, animate,
   plot, slice, compare — without the model's library present.
3. **G3 — Model-agnostic configurator**: schema-driven argument editors from
   JSON Schemas + component-supplied editors via `IUIProvider`.
4. **G4 — GIS-capable**: CRS-aware map canvas, basemaps, GDAL vector/raster
   import, spatial data items rendered as layers (as in openswmm.engine's GUI).
5. **G5 — 3D-native rendering**: the visualization surface renders layered
   3D meshes (sigma grids, TINs, extruded terrain) through Qt's RHI
   (`QRhiWidget`, decided at C3a — D30), with the 2D map as a QPainter view
   of the same stack (D14). *(Originally "QSG"; corrected 2026-09-19 to
   what was built.)*
6. **G6 — Meshing & model-setup tools**: GUI over SDK `Tools` (CDT, quad
   mesher, curvilinear, sigma, terrain sampling) plus BC/attribute assignment —
   sufficient to configure FVQual end-to-end.
7. **G7 — openswmm.gui-style engineering**: CMake + vcpkg + presets, C++20,
   MVC document model, offscreen ctest GUI suite, per-OS packaging.

### Non-goals (this program)

- Porting the v1 codebase file-by-file (it is a donor of concepts, not code).
- New meshing *algorithms* in the GUI (SDK `Tools` only; gaps are SDK work).
- A web/remote frontend; distributed-run *monitoring* beyond launch + status.
- Preserving HydroCoupleVis (retired — superseded by the in-app 3D view).
- Writing FVQual's HydroCouple component (tracked in FVQual's own plan; E6
  here only defines the interface expectations Composer has of it).

---

## 3. Design decisions

**D1 — Clean scaffold, donor port.** New CMake/vcpkg scaffold modeled on
openswmm.gui; qmake and the `.pro` files retire. The old app's *concepts*
(canvas, status model, argument dialog flow) are ported deliberately onto the
v2 stack rather than migrated line-by-line — the interface break (Qt types,
signals, data plane, XML) touches essentially every one of the 16.5k lines.
*Rejected:* incremental in-place migration — there is no compiling intermediate
state worth preserving, and no test suite to protect it.

**D2 — The project format is Composition Spec v1.** Composer's native document
is the SDK's JSON/YAML composition document, validated against the installed
schemas before every apply, plus a Composer-owned **sidecar presentation file**
(`<name>.composer.json`: canvas positions, view state, layer styling) so the
composition itself stays host-neutral and diff-clean. A one-way `.hcp` XML
importer covers legacy projects. *Rejected:* embedding presentation inside the
composition document (pollutes a spec other hosts consume); keeping XML alive
(the SDK plan explicitly ended it).

**D3 — Components load via dlopen + C entry point.** Qt plugins are dead — v2
components have no QObject to load. Composer defines the convention:
components export `extern "C" HydroCouple::IComponentInfo *hydrocouple_component_info()`
(name to be finalized with the SDK), and a `ComponentLibraryLoader` +
`ComponentRegistry` in Composer wraps `dlopen`/`LoadLibrary`, feeds
`ModelInitializer`'s `ComponentResolver`, and records provenance (path, hash,
version) in the run. The loader/registry is written to be **upstreamable to
the SDK** later (the SDK's own docs anticipate "static registry, dlopen'd
libraries"). ABI reality is documented, not hidden: C++ interfaces across a
`dlopen` boundary require same-toolchain builds; the registry records and
checks compiler/version stamps from the component info.

**D4 — One Qt bridge for ISignal/ISlot.** A small `qtbridge` module owns the
only crossing between component callbacks and Qt: templated slot adapters that
capture events and marshal them to the GUI thread via queued
`QMetaObject::invokeMethod`. Component status, data-item changes, and workflow
progress all arrive through it; nothing else in the GUI touches `ISlot`
directly. Components execute on worker threads (`SimulationManager`), with the
workflows' cooperative pause/stop wired to the UI.

**D5 — 3D via custom RHI geometry, not Qt Quick 3D.** *(As executed: the 2D
map is QPainter (D14) and the 3D scene is `QRhi` (D30), not QSG — this
decision's original "QSG/QRhi" wording predates C1b and C3a.)* The renderer
ports openswmm.gui's classification/color-ramp/legend pipeline and adds a 3D
scene: perspective/orbit camera, Z from layer interfaces (sigma) or vertex Z
(TIN), vertical exaggeration, per-layer draping. The 2D map and the 3D scene
are two views of one layer stack — one theming pipeline, two cameras.
*Rejected:* Qt Quick 3D — its material/scene
model fights data-driven scalar theming on million-cell meshes and would strand
the openswmm.gui shader/material investment (qsb scalar-fill materials
generalize; QQ3D would replace them).

**D6 — Results access only through manifests.** The results viewer consumes
`RunManifest` + `ResultsModelComponent`/`ResultReader` exclusively. No
per-model file readers in the GUI, ever — if a format can't be reached through
the manifest catalog, the fix belongs in the SDK's writers/readers. This is
what makes the viewer model-agnostic by construction.

**D7 — Meshing UI is a veneer.** Domain/PSLG editing, parameter forms, preview,
and attribute/BC assignment live in the GUI; triangulation, quad meshing,
curvilinear/sigma grids, and terrain sampling are SDK `Tools` calls emitting
UGRID meshes that the writers and FVQual consume directly. openswmm.gui's
vendored Shewchuk Triangle stays where it is.

**D8 — MVC document model.** A single `CompositionDocument` (wrapping the spec
JSON + presentation sidecar) is the source of truth; canvas, argument editors,
map layers, and property panels are views with a shared `QUndoStack`. The same
data edited from different UIs stays synchronized by construction (per
CLAUDE.md 5.1).

**D13 — The toolbar is openswmm.gui's ribbon, ported in part.** Composer uses
the same tabbed ribbon: pages of captioned groups whose faces are large
icon-over-label buttons, at the shared metrics (`kRibbonRowHeight` 100,
32 px Full / 24 px Compact icons, caption in the hint-text token, trailing
rule closing each group). **Not ported:** openswmm.gui's responsive layout
solver, width compactor and last-used split buttons — roughly 3.7k lines that
exist to degrade a very dense toolbar gracefully on narrow windows. Composer's
ribbon is a fraction of that size, so that machinery would be complexity
carried for a problem it does not yet have; the Full/Compact switch is kept
because the appearance depends on it, and the solver can follow if the ribbon
grows dense enough to need it. Icons are currently `QStyle` standard pixmaps —
placeholders with correct semantics, replaceable by a designed set without
touching the ribbon code.

**D12 — The chrome is openswmm.gui's, by copied values.** Composer installs the
Fusion style with the *same token palette and overlay* as openswmm.gui, with the
token values copied from its `ui/theme/themetokens.h` rather than re-invented —
"looks similar" and "is the same" are different claims, and copied values stay
the same through future adjustments to either side. Fusion is chosen over the
native styles deliberately: it renders identically on macOS, Windows and Linux,
so a palette verified once holds everywhere and dock/graphics chrome does not
drift between platforms. Mode follows the OS live and is persisted; offscreen
platforms report an unknown scheme, so light is the floor and headless renders
do not depend on the host's appearance. The full openswmm.gui theme subsystem
(icon factory, preferences integration) is **not** ported — only the appearance
contract.

**D10 — `QT_NO_KEYWORDS` project-wide (forced, not stylistic).** HydroCouple's
`ISignal` declares `virtual void emit(Args... args) = 0`. Qt defines `emit` as
an empty macro, so any translation unit including both Qt and `hydrocouple.h`
rewrites that to `virtual void (Args...)` and fails with *"field has incomplete
type 'void'"*. Composer is by definition where Qt and the interfaces meet, so
the Qt keywords are disabled globally and the `Q_`-prefixed forms used
instead (`Q_SIGNALS:`, `Q_SLOTS:`, `Q_EMIT`). Defined `PUBLIC` on
`composer_core` so app and tests inherit it — a target that missed the define
would fail only on first contact with an interface header. *Rejected:*
`#undef emit` fences around interface includes — one missed include site
reintroduces the break, and the fence has to be repeated in every new file.

**D11 — Composer accepts both component-loading conventions.** HydroCouple's
Python bindings already load components through an unstamped `extern "C"`
factory named `CreateComponentInfo`. Refusing those would mean a component's
loadability depended on which host opened it, so the loader tries the stamped
entry points first and falls back to the legacy factory, reporting such
libraries as *unstamped* rather than validated
(`ComponentLibrary::isUnstamped()`). The stamped convention is the target;
the fallback is the compatibility floor. Converging Python onto the stamped
form is item P1 of `PYTHON_SCRIPTING_PLAN_2026-08-24.md`.

**D9 — Reuse from openswmm.gui by adaptation, not by shared library.** The
map/render/classification subsystems are copied into Composer and adapted
(namespaces, attribute-provider seams to HydroCouple data items). Extracting a
shared "mapkit" library is explicitly deferred: openswmm.gui is under heavy
active development and a premature extraction couples both programs' release
cadences. Revisit after M3 if divergence hurts. *Cost acknowledged:* a fork of
~the render core; mitigated by keeping file/class names aligned so fixes port
mechanically.

**D37 — Behaviour from openswmm.gui, never source** *(lifted from §9,
2026-09-19)*. openswmm.gui is GPL-3.0. Composer copies token *values* (D12)
and re-implements behaviour clean-room — the welcome page (`fb24baa`), the
credential store (`e6c6727`) and the preferences dialog (U7) are the
precedents.

---

## 4. Target architecture

```
HydroCoupleComposer/
├── CMakeLists.txt, CMakePresets.json, vcpkg.json      (A1)
├── include|src/
│   ├── core/          version, logging, paths, settings
│   ├── plugins/       ComponentLibraryLoader, ComponentRegistry (D3)
│   ├── qtbridge/      ISignal→Qt adapters, GUI-thread marshaling (D4)
│   ├── project/       CompositionDocument, presentation sidecar,
│   │                  schema validation, .hcp importer, undo commands (D2/D8)
│   ├── canvas/        composition graph editor (QGraphicsView)      (B)
│   ├── configurator/  schema-driven argument editors, IUIProvider host (B)
│   ├── simulation/    SimulationManager, workflow control, status model (B)
│   ├── map/           canvas, cameras (ortho 2D / perspective 3D), CRS,
│   │                  snap, selection                                (C)
│   ├── render/        isublayer + sublayers + materials + classification
│   │                  + legend (ported), 3D geometry nodes            (C)
│   ├── layers/        HydroCouple spatial-item layers, UGRID mesh layer,
│   │                  basemaps, GDAL import                           (C)
│   ├── results/       run browser, manifest model, animation, plots  (D)
│   ├── meshing/       domain editor, tool dialogs, attribute/BC assign (E)
│   └── ui/            main window, panels, dialogs
├── forms/, resources/
├── tests/gui/         offscreen ctest suite                          (F, ongoing)
└── plans/, docs/
```

Dependency flow: `HydroCouple` (headers) ← `HydroCoupleSDK` (find_package,
features `tools,netcdf,hdf5,geopackage[,gdal,mpi]`) ← Composer. Qt6 + GDAL
via vcpkg, mirroring openswmm.gui's manifest/preset layout.

---

## 5. Work plan

Each item lists **→ verify:** the check that closes it. GUI items get an
offscreen ctest where instantiable (lesson from Y1: a dialog no test
instantiates is an observer hole).

### Phase A — Platform (scaffold, loading, bridge, document)

- **A1. Build scaffold. — ✅ DONE, verified 2026-08-24.** New top-level CMakeLists + `vcpkg.json`
  (qt-less core deps: hydrocouplesdk via find_package/overlay port, gdal,
  nlohmann-json, gtest) + `CMakePresets.json` (default/Darwin/Linux/Windows ×
  debug), Qt6 module set as in openswmm.gui (incl. ShaderTools), `version.h.in`,
  CI skeleton. Old qmake files removed in the same change.
  → verify: all presets configure; a `main.cpp` shell app builds and launches
  offscreen on macOS CI; `ctest` runs a trivial GUI smoke test.
  → **result:** Darwin preset configures; app builds and launches offscreen
  (alive at SIGALRM, clean); `ctest` green with 4 behavioural cases
  (`test_shell_smoke`), one of which *calls* an exported SDK symbol so a
  headers-only/mis-resolved dependency fails at A1 rather than later. The v2
  entry point lives at `src/app/main.cpp`; the legacy qmake tree and
  `src/main.cpp` are untouched pending the repo-strategy answer (Q1).
  Linux/Windows presets are authored but unexercised (host-conditional) —
  they stay unverified until CI runs them (F1).
- **A2. Component loading. — ✅ DONE, verified 2026-08-24** (registry settings
  UI deferred to the preferences dialog — **delivered in U7 as the
  Components page, 2026-09-19**; metadata cache and content hashing deferred to B3, where
  run provenance first needs them).
  Convention settled (Q2): a component library exports two C symbols,
  `hydrocouple_component_abi_v1` (pure C, safe across any mismatch) and
  `hydrocouple_component_info_v1`; `HYDROCOUPLE_DECLARE_COMPONENT(InfoType)`
  emits both. Loading is staged — open, read the stamp, compare, and only then
  call into C++ — so a mismatched library is never asked to run C++ code.
  `include/plugins/componentabi.h` is kept Qt-free and Composer-free so it can
  be upstreamed into the SDK unchanged. `ComponentLibraryLoader` (dlopen/LoadLibrary,
  entry-point convention per D3), `ComponentRegistry` (search paths, metadata
  cache, toolchain stamps), settings UI for library directories.
  → verify: a test component library (built in-tree from SDK
  `AbstractModelComponent`) loads, exposes `IModelComponentInfo`, instantiates,
  and unloads clean under ASan; a wrong-ABI/garbage library is rejected with a
  diagnostic, not a crash.
  → **result:** 8/8 green, and green again under ASan (`detect_leaks=0`;
  LeakSanitizer is unavailable on Darwin/arm64). Three real fixture libraries
  are built in-tree: `testcomponent` (SDK-derived — the test drives
  `initialize()`/`validate()` through it, proving virtual dispatch across the
  boundary, not merely a successful `dlopen`), `notacomponent` (a valid library
  that is not a component), and `badabicomponent` (exports both entry points
  but reports `iface=99`). Falsified rather than assumed: `nm` confirms
  `badabicomponent` really does export `hydrocouple_component_info_v1`, so the
  ABI test exercises the stamp-mismatch path and not, accidentally, the
  missing-symbol path — the two rejection tests are genuinely distinct. The
  bad-ABI fixture's info entry point returns a garbage pointer, so a passing
  test is itself evidence the loader never called it.
- **A3. Qt bridge. — ✅ DONE, verified 2026-08-24.**
  `QueuedSlot<Payload, Args...>` is the single ISlot implementation in
  Composer: it snapshots the payload on the emitting thread and re-delivers it
  through `QMetaObject::invokeMethod` bound to a context QObject (so a dead
  observer cancels its queued calls). `ComponentStatusObserver` publishes one
  component's status as a Qt signal. Value-changed and workflow observers reuse
  the same template and land when B2/B3 first consume them. `qtbridge` slot adapters + GUI-thread marshaling;
  status/value-changed/workflow events surface as Qt signals.
  → verify: unit test drives a component from a worker thread and asserts
  events arrive on the GUI thread, in order, without re-entrancy; disconnect
  during emit is safe.
  → **result:** 5/5 green (worker-thread delivery, ordering across 20 events,
  snapshot contents, detach mid-flight, destruction with deliveries queued).
  **The first version of the thread-affinity test was vacuous** and was caught
  by mutation: breaking the bridge to `Qt::DirectConnection` still passed,
  because `QObject::connect` defaulted to `AutoConnection` and re-queued onto
  the GUI thread by itself — the test was measuring Qt's auto-queue, not the
  bridge. The assertions now connect with `Qt::DirectConnection` so they
  observe the thread `statusChanged` is actually emitted on; the mutation now
  fails the test and reverting restores it.
  → **interface finding:** `IModelComponent` inherits two different `ISignal`
  instantiations — the status signal, and `ISignal<std::string>` via
  `IIdentity → IDescription → IPropertyChanged` — so `connect`/`disconnect`
  are ambiguous and must be qualified by casting to the intended signal base.
  Every future observer hits this.
- **A4. CompositionDocument (MVC core). — ✅ DONE, verified 2026-08-24.**
  `CompositionDocument` owns a `CompositionSpec` plus the `Presentation`
  sidecar and is the single source of truth; views observe its change signals
  rather than holding copies. Commands snapshot the **whole spec** before and
  after each edit rather than computing fine-grained inverses — a composition
  is a small document, and whole-spec swaps make undo exactly right for edits
  with collateral effects (removing a component takes its connections with it,
  and undo brings them back). Presentation-only edits are separate commands, so
  dragging a box does not dirty the composition; consecutive moves merge into
  one undo step. Document wrapping composition JSON +
  presentation sidecar; schema validation (installed schemas) with error
  surfacing; `QUndoStack` command set (add/remove component, connect, set
  argument); dirty tracking; save/load round-trip via `ModelInitializer`'s
  symmetric serialization.
  → verify: round-trip test — load the SDK's `serial_coupling` example
  composition, mutate via commands, save, reload, byte-stable on re-save;
  invalid documents produce actionable errors, never partial applies.
  → **result:** 12/12 green. The round-trip fixture is the SDK's own example,
  not a document authored here, so the test cannot agree with a mistaken idea
  of the format. **A blind spot was found and closed while checking the test
  rather than trusting it:** re-save byte-stability would still have held if
  the round-trip silently dropped argument payloads, since both saves would
  drop them identically — the test now asserts the payloads survive and that
  the written file contains real content. Rejection paths covered: malformed
  JSON, a dangling connection endpoint, and — the "never partial applies"
  requirement — a failed load leaving the already-open document byte-identical.
  YAML input is not yet accepted (JSON only); it arrives with `ModelInitializer`
  in B3.
- **A5. `.hcp` importer. — ✅ DONE, verified 2026-08-24.** One-way XML → Composition Spec v1 + sidecar
  (positions preserved).
  → verify: the repo's legacy example `.hcp` files import; unsupported
  constructs are reported per-node, not dropped silently.
  → **result:** 9/9 green. **Caveat on confidence: no `.hcp` files survived in
  the repository**, so the fixtures in `tests/fixtures/legacy/` are
  reconstructed from the v1 writer itself (`HydroCoupleProject::onSaveProject`,
  `GModelComponent::write*`, `GOutput`, `GInput`, `GAdaptedOutput`). The
  importer is proven against the format as the v1 code emits it, not against
  field projects — re-verify if a real corpus turns up.
  → **the governing rule: never invent what v1 did not record.** v1 argument
  payloads are opaque text in each component's own dialect, while v2 payloads
  are JSON passed verbatim to `IArgument::initialize()`; there is no general
  translation, so every argument becomes a reported issue *carrying its
  original value* and no payload is fabricated. Same for compute-resource
  allocations and v1 workflow-component libraries.
  → **a violation of that rule was caught by testing the output, not the
  input.** The importer first mapped v1's `IsTrigger` onto a pull-driven
  workflow — but the schema requires a pull-driven workflow to name a trigger
  *input* too, which v1 never recorded, so the emitted document failed to
  load. The test that round-trips the imported composition back through
  `CompositionDocument` caught it. The trigger is now reported, not guessed.
  Structure that *does* convert faithfully: components (library and
  component-info references, captions), index-based connections resolved to
  synthesised ids, adapted-output chains, and canvas positions into the
  sidecar.

### Phase B — Composition editor & execution

- **B1. Canvas. — ✅ DONE, verified 2026-08-24.** Node/connection graph editor: components, inputs/outputs/
  multi-inputs, adapted-output chains as first-class edges; drag from a
  component palette (registry-fed); selection/copy/delete; layout persistence
  in the sidecar. Port the v1 canvas interaction model; fire dialogs from
  mouse **release** (macOS modal-from-press freeze, known gotcha).
  → verify: offscreen test builds a two-component composition by simulated
  interaction and the document JSON equals the hand-written equivalent.
  → **result:** 9/9 green. The decisive test performs two palette drops and a
  real port-to-port drag (press/move/release delivered as scene mouse events)
  and compares the resulting document with a hand-written one — asserting on
  the *document*, not on scene internals, because the document is what every
  other view and host reads. Node drags are likewise driven as real drags.
  → **architectural finding: a component's ports do not exist until it is
  initialized.** The interface states that Inputs and Outputs are set only once
  `initialize()` completes, and that `arguments()` is the sole property valid
  before then — so the canvas cannot draw a single port from the specification
  alone. `ComponentInstances` (new, `project/`) realises live instances from
  the registry, initializes them, and surfaces per-component failures; the
  canvas draws a component whose library is missing as an unavailable box
  rather than refusing to open the document. B3 and the configurator reuse it.
  → the scene is a pure view: it rebuilds from document signals and pushes
  every edit — move, connect, delete — as an undoable command, so an edit made
  anywhere else appears on the canvas and vice versa.
- **B2. Configurator. — ✅ DONE, verified 2026-08-24.** Schema-driven argument forms (JSON Schema → widgets:
  scalars, enums, arrays, file refs, tables for 1D/2D typed arguments) with a
  synchronized raw JSON/YAML editor pane (validation on the fly); per-argument
  `ArgumentInputType` handling (inline vs file vs URL); `IUIProvider` hook —
  if the component advertises an editor, offer it (opaque pointer = parent
  `QWidget*`).
  → verify: contract test hydrates forms from a fixture component's
  arguments, edits every kind, and asserts `serialize()` round-trip equality
  (options-hydration-contract pattern from openswmm.gui).
  → **result:** 13/13 green, covering an integer, free text, a categorical, a
  rank-1 array and a rank-2 grid.
  → **plan correction — forms are introspection-driven, not schema-driven.**
  This item was written assuming JSON Schema would generate the widgets, but
  there is no per-argument schema anywhere: Composition Spec v1 declares each
  component's `arguments` object `additionalProperties: true` and passes it
  verbatim to `IArgument::initialize()`, precisely because a payload's shape is
  the component's business. Forms are therefore generated from what the
  argument advertises through the v2 data plane — rank and `shape()`,
  `DataKind`, and the value definition. That is a *better* source: an
  `IQuality` value definition enumerates its permitted categories, so a
  categorical argument becomes a combo box the component itself defined, which
  no document-level schema could have known.
  → **payloads are never interpreted by Composer**: read with
  `serialize(JSON, …)`, written with `initialize(…, JSON, …)`, and offered to
  the live component *before* the document records them — so a value the model
  refuses cannot enter the document. The raw JSON pane is all-or-nothing for
  the same reason.
  → **a real bug the contract caught:** rank-2 payloads serialise as nested
  rows, not a flattened run, and the table editor had assumed flat indexing.
  The component rejected the flat payload with "Rows must be arrays of equal
  length"; the editor now reads whichever form the component produced.
  → `IUIProvider` is honoured: a component advertising its own editor is
  offered it (passing the panel as the opaque parent pointer), and the button
  stays hidden when it does not.
- **B3. SimulationManager. — ✅ DONE, verified 2026-08-24 (including B3b
  recording).** Compose → validate → prepare → run through
  `PullDrivenWorkflow`/`TimeSteppedWorkflow` on worker threads; pause/stop
  buttons wired to cooperative sync points; status/progress panel
  (`modelstatusitemmodel` concept, v2 events); error queue (`errors()`)
  surfaced; output-writer and manifest-path configuration; `run|open|resume`
  execution modes per component in the UI.
  → verify: runs the SDK `serial_coupling` example end-to-end from the GUI
  test harness; pause/resume/stop leave states consistent; the run manifest
  appears and validates.
  → **result:** 9/9 green — a two-component composition runs to completion on
  a worker thread, reports steps and state transitions on the GUI thread, and
  refuses bad compositions *before* starting a thread (empty composition,
  uncreatable component, a second concurrent start).
  → **the run owns its own component instances.** It never drives the ones the
  canvas and configurator introspect: those are read on the GUI thread, and a
  running workflow mutates its components continuously. Everything that can
  fail early — instantiation, applying the document via `ModelInitializer`,
  wiring, `validate()` — happens synchronously in `start()`, so a composition
  that cannot run says why immediately rather than failing on a worker thread.
  → **a real concurrency bug the tests caught:** `requestResume()` can arrive
  *before* the workflow has reached `Paused`, consuming the resume and wedging
  the run (it hung for the full 15 s timeout). Pause is now tracked as a
  user *intent* the worker re-applies whenever it observes `Paused`, which
  makes resume idempotent and the ordering irrelevant. The fixture's step count
  was raised so a pause can actually be caught mid-run — at two steps the test
  would have been vacuous, passing without ever pausing.
  → **B3b — recording: ✅ DONE.** 12/12 green. A composition naming writers and
  a run manifest now produces both: an `IOThread` carries per-step snapshots to
  the writers, and the manifest is written *after* `finish()`, because writer
  catalogues are only valid once the IO thread has finalised. Recording is
  model-agnostic — `Snapshot::capture` takes a plain `IComponentDataItem`, so
  every component's outputs are captured without any component-specific
  knowledge. The test asserts the CSV holds data rows and that the manifest
  actually *names* the recorded output and its artefact, not merely that a file
  exists — a manifest that catalogues nothing is no use to D1.
  → **writer support is bounded, and says so.** `csv` is built; the WriterSpec
  type strings are specified by the SDK, so the mapping is not guesswork. The
  `netcdf_ugrid`, `hdf5_ugrid` and `geopackage` writers are constructed with a
  `MeshDefinition` describing the grid their values live on, which a
  composition of non-spatial components cannot supply — asking for one is
  **refused up front with that explanation**, never accepted and then silently
  producing nothing. They become available with the spatial data items of
  phases C and E.
- **B4. Command-line parity. — ✅ DONE, verified 2026-08-24.** `--run composition.yaml` headless path (reuses
  A2/A4/B3 sans widgets) so CI and users can execute without the window.
  → verify: headless run of the example equals the SDK CLI example's outputs.
  → **result:** `HydroCoupleComposer --run <composition> --components <dir>`
  runs without a display and exits non-zero on failure; a ctest drives the
  *shipped binary* as a subprocess, because "the application runs a
  composition headlessly" is not a property a library-level test can
  demonstrate. It asserts the artefacts hold content and that the manifest
  catalogues the recorded output — 39 kB of CSV plus a manifest, produced
  fresh each run. The headless path reuses the same registry, document and
  SimulationManager the window uses; a separate runner would be a second
  implementation to keep honest.
  → **the window is now assembled** (this was outstanding until B4): component
  palette, composition canvas, argument configurator, run toolbar with
  pause/resume/stop, diagnostics log, and File ▸ New/Open/Save/Import .hcp with
  working undo/redo. `tools/screenshot.cpp` renders it headlessly to a PNG so
  the UI can be reviewed in CI without a display.

- **B5. Typed argument editors, and arguments bound to component outputs.**
  *(new scope, added 2026-08-25 at the user's request; B5b delivered via
  the provider pipeline; **B5a is executed as Phase U2** — standalone typed
  dialogs — with SDK prerequisite U2-S, see §5 Phase U)*

  B2 delivered one generic form per `ArgumentEditorKind` — Categorical,
  Number, Integer, Boolean, Text, FilePath, Table, Raw. That covers *entry*
  but not *meaning*: a time step, a coordinate reference system, a start and
  end date, a rating curve and a lookup table are all "a number" or "a table"
  today, and the editor cannot help with any of them. Two pieces:

  - **B5a. Specialized editors per argument type.** Widen
    `ArgumentEditorKind` and give each kind an editor that knows what the
    value *is*: a quantity spin box that shows and converts the argument's
    `IUnit` (entered in one unit, stored in the component's); a date/time and
    a duration editor; a CRS picker reusing C1a's `SpatialReference`; a file
    reference that offers the argument's own filters and validates existence;
    a curve/series editor with a plot beside the table; a colour and an
    enum-flags editor. Kind is still decided by *introspection*, per B2's
    correction — the value definition's `DataKind`, its `IQuantity`/`IQuality`
    and its dimensions — never by a document-level schema, and never by
    matching an argument's id or caption against a list of known names, which
    would silently mis-type an argument that happened to share a caption.
    → verify: the B2 hydration contract extended to every new kind, asserting
    `serialize()` round-trip equality; plus a unit-conversion case where the
    value entered and the value stored deliberately differ, since an editor
    that ignored the unit would round-trip perfectly.

  - **B5b. Arguments bound to another component's output.** An argument may
    take its value from a component rather than from the user. This is *not*
    an exchange-item connection: those move values between running models
    each time step, whereas this resolves once, before `initialize()`, and is
    exactly how a mesh generator hands a mesh to a solver, or a calibration
    step hands a parameter set to the model it calibrated.

    **REDESIGNED 2026-09-03 by the provider-pipeline program**
    (`HydroCouple/plans/sdk/PROVIDER_PIPELINE_PLAN_2026-09-03.md`), which
    supersedes the original sketch on two points and answers Q9-Q11:
    - The binding is **SDK-owned `@from`** in Composition Spec **v1.1**
      (`{"@from": {"component", "output", "selector"}}`, joining the
      `@uri`/`@ref` sigil family), not a Composer `$from` convention —
      schema-visible so `CompositionSpec::parse` builds the DAG and refuses
      cycles naming the edge, and headless `--run`/Python resolve
      identically (program slice S1.1).
    - Resolution is a **staged pre-pass in SDK `ModelInitializer`** (S1.2):
      providers run to `Finished` before a consumer initializes; the
      consumer argument reads the live output through
      `IArgument::initialize(const IComponentDataItem&)`; `serialize()`
      re-emits the *reference*, never the baked value. Composer keeps only
      the UX: the configurator's "From component output…" picker (S4.1,
      type-filtered by `validComponentDataItemTypes()`) and distinct canvas
      binding edges (S4.2).
    - **Q9 answered:** provenance-on-load, value-after-provider-runs.
      Opening a composition never executes anything; the configurator shows
      a binding chip (provider + output + status). Corollary contract:
      consumers keep valid defaults while a bound argument is unresolved,
      and providers do no heavy work in `initialize()`.
    - **Q10 answered:** whole output item only; `selector` reserved in the
      schema and diagnosed as unimplemented (the ExecutionMode::Resume
      pattern).
    - **Q11 answered:** dangling bindings survive as repairable, matching
      canvas connections — red badge, Run blocked, `validateDocument` error
      downgraded to a warning at edit time.

    → verify (unchanged in spirit): a two-component fixture where the
    second's argument is bound to the first's output; the second receives
    the produced value and the document round-trips the *reference*, not
    the resolved value; a cycle is refused at document level with the
    offending edge named.

- **B6. Workflow/execution UI + run staging.** *(added 2026-09-03, program
  slices S4.3/S4.4)* The first UI ever for `WorkflowSpec`: strategy /
  trigger / iterationsPerGroup / maxSteps, per-component ExecutionMode
  (run|open|resume), and a staged-order preview via the SDK's
  `bindingStages()`. SimulationManager forwards ModelInitializer's new
  progress callback so the run panel shows stage progression (providers →
  coupled run → recording).

### Phase C — GIS map + 3D QSG view

- **C1. Map substrate port.** *(delivered in slices; C1a done)*
  - **C1a — coordinate reference systems. ✅ DONE, verified 2026-08-24.**
    `SpatialReference` implements HydroCouple's own
    `Spatial::ISpatialReferenceSystem` over GDAL/OGR rather than introducing a
    parallel CRS type — spatial data items already carry that interface, so
    layers, components and the map describe their CRS identically with no
    conversion layer between them. `CoordinateTransform` is a separate object
    with an explicit lifetime because PROJ resolves an operation pipeline on
    construction, which is far too costly to repeat per point; bulk transform
    is one call for a whole run, which is what pan and zoom need.
    → **result:** 7/7 green. Reprojection is checked against **known answers**,
    not round-trips alone — a transform that did nothing would round-trip
    perfectly. Includes an explicit axis-order assertion: without
    `OAMS_TRADITIONAL_GIS_ORDER`, PROJ honours each CRS's declared axis order
    and EPSG:4326 arrives as (latitude, longitude), silently transposing every
    map. Points that fail to transform are left unchanged and counted rather
    than dropped, since dropping a vertex deforms a geometry instead of
    reporting a problem.
  - **C1b — map canvas, layer stack and layer tree. ✅ DONE, verified 2026-08-25.**
    Four pieces: `MapTransform` (world↔pixel), `MapLayer` (abstract drawable),
    `LayerStackModel` (the owning, ordered set of layers) and `MapCanvas`
    (the widget), with `LayerTreePanel` as the second view of the stack.
    The stack is the single owner: the canvas and the tree hold no layer state,
    so a layer toggled in the tree is redrawn without either widget knowing
    about the other. Row 0 is the top of the stack, as every GIS layer tree
    shows it, so drawing walks the rows backwards.
    → **result:** 3 new suites (12 + 15 + 15 tests) plus 3 shell tests; 13/13
    ctest green, docs 0 warnings. Capture: `artifacts/composer-map.png`.

    Decisions taken here:
    - **D14 — QPainter, not QSG, for the 2D map.** The QSG/QRhi path arrives
      with the 3D scene in C3 where it earns its complexity; using it for a
      handful of vector layers would mean maintaining a shader pipeline to
      draw what QPainter draws in a line.
    - **D15 — the base layer stays thin.** openswmm.gui's `OpenSWMMVisLayer`
      grew labelling, masks, joins, diagrams and temporal config onto the base
      class; each made every layer type pay for a feature most of them do not
      have. Composer's base carries only what the stack and the canvas need.
    - **D16 — C1b is single-CRS for geometry.** The canvas reprojects layer
      *extents* into the map CRS (that is where a mixed-CRS stack otherwise
      produces a nonsense zoom); reprojecting layer *geometry* belongs with the
      concrete layers in C1d/C2, which is also where the plan verifies
      pan/zoom under reprojection.
    - **D17 — the transform's viewport is synchronised lazily,** from every
      entry point that reads or moves the view, because Qt does not deliver a
      resize event to a widget that has never been shown — precisely the case
      that needs it, a map framed while its tab was still hidden.

    Two bugs the falsification pass caught, both of which the first green run
    had hidden:
    - The canvas frames the first layer to arrive, which made the
      *Zoom to Full Extent* shell assertion **vacuous** — the view was already
      correct before the action ran. Fixed by looking elsewhere first.
    - A framing chosen before the map tab's first layout was **lost**:
      `resizeEvent` set the viewport itself, leaving the re-framing with
      nothing to notice. The map drew at ~1/15 scale in the corner of the tab.
      Only the shell-level test reproduces it — a hidden widget never gets the
      resize event that triggers the bug.
  - **C1c — classification, colour ramps, legend and labelling. ✅ DONE,
    verified 2026-08-25.**
    `ColorRamp` (six perceptual built-ins), `Classification` (equal interval,
    quantile, Fisher-Jenks natural breaks, manual), `IAttributeProvider`,
    `LayerStyle` (single / graduated / categorised, plus the layer's
    `LabelConfig`), `LabelPainter` with a per-frame `LabelCollisionMap`, and
    `LayerStyleDialog` as the editor. The legend is **derived from the style
    on every read**, never stored — that is what stops a map and its legend
    disagreeing — and appears as checkable child rows in the layer tree, so a
    class switched off there disappears from the map with no second
    visibility list to keep in step.
    → **result:** 3 new suites (26 + 7 + 7 tests) plus 8 layer-stack and 2
    canvas tests; 16/16 ctest green, docs 0 warnings. Capture:
    `artifacts/composer-styling.png`.

    Decisions taken here:
    - **D18 — LayerStackModel became a two-level QAbstractItemModel.** The
      panel was built on a QTreeView in C1b precisely so this could arrive
      without replacing the view. Legend rows carry their owning layer as the
      index's internal pointer; layer rows carry none.
    - **D19 — labelling lives inside LayerStyle**, not beside it. Both answer
      the same question — how this layer appears — and a reader changing one
      almost always wants the other to hand. It also avoids a second
      nullptr-returning virtual on the thin base (D15).
    - **D20 — natural breaks samples at 512 values on a deterministic
      stride.** Jenks is O(k·n²), so a large layer would otherwise stall the
      UI; a *random* sample would classify the same layer differently every
      time it was opened.
    - **D21 — an invalid QColor is how a style says "do not draw".** Falling
      back to the base symbol would put an unclassified feature on the map
      looking like a category the legend never mentions.

    Three vacuous gates the falsification pass caught, one of which was a
    code defect rather than a test defect:
    - The natural-breaks test asserted only that the boundary fell "in the
      gap", which **equal interval also satisfies** on that data. Now it
      asserts the Jenks edge lands exactly on the cluster and differs from
      the equal-interval answer.
    - Nothing covered a value that **no category matches**, so the
      "don't draw it" rule was untested.
    - The `rebuild()` guard for a missing attribute was **redundant** — the
      general path already refuses — and it additionally **reset the
      classification**, throwing away the ramp and class count the user had
      chosen. Deleted rather than tested.
  - **C1d — basemaps and GDAL vector/raster import. ✅ DONE, verified
    2026-08-25.**
    `TileGrid` (the slippy-map scheme as arithmetic), `TileLayer` over an
    injectable `ITileSource`, `NetworkTileSource` with four built-in
    providers, `GdalVectorLayer` (OGR, points/lines/polygons, attributes,
    labels) and `GdalRasterLayer` (windowed reads, ramp shading, warped-VRT
    reprojection). Add Vector / Add Raster on the Map ribbon; a Basemap
    chooser under View.
    → **result:** 3 new suites (10 + 9 + 15 tests) plus 2 shell tests; 19/19
    ctest green, docs 0 warnings. Capture: `artifacts/composer-gis.png`.

    Decisions taken here:
    - **D22 — XYZ tiles now, WMTS capabilities later.** Every provider in
      practice — OSM, the commercial ones, and WMTS's own
      GoogleMapsCompatible matrix set — is the same Web Mercator grid.
      Parsing WMTS capabilities XML buys a discovery step, not a new
      capability, and is deferred until a dataset needs it.
    - **D23 — the tile source is an interface.** A basemap test that needed
      the internet would fail for reasons unrelated to the code and would be
      switched off within a week.
    - **D24 — attribution is data the layer carries,** surfaced through
      `MapLayer::attribution()` and drawn by the canvas over everything else.
      Every free provider makes it a condition of use, so it cannot be
      something a caller might remember to add.
    - **D25 — raster reprojection uses GDAL's warped VRT.** Resampling a grid
      correctly is solved; it reprojects lazily, so opening a large raster in
      another CRS costs nothing until pixels are asked for.
    - **D26 — `expandTo`/`overlaps` replace `QRectF::united`/`intersects`**
      for all extent maths. See below.

    **A degenerate-rectangle trap, twice.** `QRectF::united()` *discards* a
    rectangle it considers null, and a zero-area rectangle is null — so
    uniting point-feature bounds one at a time left the extent of the **last
    point**. `QRectF::intersects()` likewise answers false for a zero-area
    rectangle, so every point feature was **culled before being drawn**. Both
    are silent and look like projection bugs. `map/extentmath.h` now holds
    the explicit min/max forms, and the probe layer in the tests carried the
    same bug.

    **Five vacuous gates, each a test that could not distinguish:**
    - The seam test built its **own** painter, so it rendered without the
      antialiasing `MapCanvas` enables — and seams only appear when
      antialiasing meets a fractional tile boundary. Measured directly with a
      throwaway probe: 0 uncovered pixels without antialiasing, 1001 with it.
      The test now renders through the canvas.
    - Reprojection was asserted by "something is drawn at the origin", which
      unreprojected degrees also satisfy — everything collapses onto that one
      pixel. Now asserts features land 111 km apart.
    - The map-CRS broadcast was asserted via the canvas's extent, which the
      canvas reprojects itself; only drawn **geometry** depends on the layer
      being told.
    - The read-window and no-data raster tests used a fixture smaller than
      the viewport and a no-data value **no pixel held**.
    - Polygon translucency was asserted by brightness, and the sampled
      polygon's class colour was light either way. Now drawn over two
      backgrounds: an opaque fill gives the same colour on both.

  *(C1's original scope line, kept for the record:)* Bring over
  openswmm.gui's map canvas + renderer + classification/color-ramp/legend +
  labeling + snap + CRS management (GDAL/PROJ) + WMTS/raster basemaps, with
  attribute access re-seamed to an `IAttributeProvider` over HydroCouple data
  items. Layer tree panel with per-sublayer visibility.
  → verify: existing openswmm.gui render tests adapted and green offscreen;
  basemap + vector overlay pan/zoom under reprojection.
- **C2. Spatial data-item layers. ✅ DELIVERED bar two items, verified
  2026-08-25.**
  `FeatureLayer` was extracted first so a file and a data item are the same
  thing once loaded — geometry, attributes and a style — leaving the loaders
  differing only in where features come from. `DataItemLayer` covers
  `IGeometryComponentDataItem`, `INetworkComponentDataItem` and
  `IPolyhedralSurfaceComponentDataItem`/TIN; `MeshLayer` draws an SDK
  `MeshDefinition` as faces, edges or nodes, and converts an
  `IRegularGrid2DComponentDataItem` into one. **Add Layers from Components**
  on the Map ribbon scans the composition's live instances.
  → **result:** 15 tests in a new suite; 20/20 ctest green, docs 0 warnings.
  Capture: `artifacts/composer-mesh.png`.

  Decisions taken here:
  - **D27 — a regular grid is converted to a mesh, not given its own
    renderer.** Its cells are quads; converting means it inherits
    classification, the legend and labelling unchanged. Inactive cells are
    left out — they are holes in the domain, and drawing them would show
    ground the model does not solve on.
  - **D28 — geometry comes through WKB, not a type switch.** HydroCouple
    geometries produce WKB and OGR reads it, so every type the standard
    admits — multi-parts and collections included — arrives through one
    tested path rather than a switch that grows a case per surprise.
  - **D29 — the entity axis is asked for, never assumed.** See below.

  **The SDK implemented none of the standard's spatial interfaces — now
  closed upstream.** Its `Point`, `LineString`, `Polygon`,
  `PolyhedralSurface`, `Raster` and `RegularGrid2D` were value types, and
  `EnvelopeAdapter` was the only implementation of any spatial interface, so
  a component publishing a geometry data item had to implement `IGeometry`
  itself — all 32 methods. **Fixed in the SDK** (see below): Composer's tests
  now build their geometry from the SDK's `PointAdapter`/`PolygonAdapter`,
  so the data-item layers are verified against what a real component
  publishes rather than against a viewer-shaped imitation of it.

  **A trap the standard's own ordering sets.** The SDK's spatiotemporal items
  declare dimensions `{time, geometries}` — **time first** — while a purely
  spatial item puts the entity axis at 0. Reading "the leading dimension"
  would colour every feature by a time index and look entirely plausible. The
  entity axis is now asked of the item (`geometryDimension()`,
  `edgeDimension()`, `patchDimension()`, …) and matched by pointer identity;
  every other axis is read at its **last** index, which for time is the most
  recent step.

  **UGRID file reading — delivered, through the SDK.** `MeshLayer::
  fromUGRIDFile()` reads a mesh through the SDK's new reader, so Composer
  needs no NetCDF dependency of its own: the SDK already links it, and
  `MeshLayer::ugridSupported()` asks at runtime so the **Add Mesh** command is
  offered only when it would work. Verified end to end — the fixture is
  written by the SDK's own `NetCDFUGRIDWriter` and read back through the
  reader, so the pair are proven to agree rather than each proven against a
  file this project invented.

  **Left out, deliberately:**
  - **Raster data items** (`IRasterComponentDataItem`). Needs the image path
    generalised away from GDAL's RasterIO, and nothing yet implements
    `IRaster`/`IRasterBand` — the SDK's `Raster` is a value type, and unlike
    the geometry family it was not adapted, because a raster data item is
    rare in practice and the image path belongs with the results viewer in
    phase D where raster results actually appear. File rasters are already
    covered by C1d's `GdalRasterLayer`.

- **S. HydroCoupleSDK advance (2026-08-25).** Two gaps C2 exposed, closed in
  the SDK rather than worked around in Composer:
  - **`spatial/geometryadapters.h` + `spatial/meshadapters.h`** — Point,
    LineString, Polygon, Vertex, Edge, Network and PolyhedralSurface adapters
    following the `EnvelopeAdapter` precedent, plus `MeshViewAdapter` exposing
    a `MeshDefinition` as the standard's `IMeshView` (the two were written to
    the same vocabulary, so it hands out spans rather than copying). The
    relational predicates need a geometry engine the SDK does not link and
    report false/nullptr rather than guessing; the WKB they do produce is what
    a caller with GEOS or GDAL uses instead.
  - **`io/ugridreader.h`** — the reader the writers lacked, driven by the
    UGRID conventions rather than by the SDK's own variable names. Honours
    `start_index` and drops `_FillValue` padding.
  - **A writer bug found by the round trip:** `NetCDFUGRIDWriter` listed only
    `_node_x` and `_node_y` in `node_coordinates`, so an elevation it wrote as
    `_node_z` was present in the file but **undiscoverable** by any
    conventions-driven reader. Fixed; elevations round-trip now.
  → **result:** 17 new SDK tests, all 11 mutations bite; SDK suite otherwise
  unchanged (4 pre-existing macOS split-HDF5 failures, present on a clean
  baseline, are untouched).
- **C3. 3D scene.** *(delivered in slices; C3a done)*
  - **C3a — camera, scene substrate and mesh surfaces. ✅ DONE, verified
    2026-08-25.** `Camera` (orbit/perspective and orthographic, framing,
    vertical exaggeration), `Bounds3D`/`SceneGeometry`, `ISceneSource`,
    `SceneRenderer`, `renderSceneToImage()`, and `SceneView` as a second view
    of the same `LayerStackModel`. `MeshLayer` supplies surfaces from its node
    elevations, coloured by the same `LayerStyle` the map reads.

    Decisions taken here:
    - **D30 — QRhi, not QSG or QOpenGLWidget.** *(was D15; renumbered 2026-09-19, C1b holds D15)* `QRhiWidget` is Qt's answer to
      3D in a widget application without Qt Quick, and `qt_add_shaders` gives
      one material source for Metal, Vulkan and D3D. QSG is a 2-D scene graph
      whose renderer owns the depth buffer for batch ordering, which a real
      camera fights; `QOpenGLWidget` would bet the long-lived part of the
      program on an API Apple froze at 4.1 in 2018. The cost is a dependency
      on `Qt6::GuiPrivate`, since Qt still ships `qrhi.h` under the private
      include path.
    - **D31 — the renderer is widget-free, and that was forced.** *(was D16)* A probe
      established that **`QRhiWidget` cannot create a device under the
      offscreen platform plugin** ("QRhi is not supported on this platform") —
      the platform the whole suite runs under — while a standalone `QRhi`
      drawing into a texture works there and produces pixel-identical output.
      So `SceneRenderer` takes a device and a target as arguments, `SceneView`
      is a thin host, and every pixel-level gate goes through
      `renderSceneToImage()`.
    - **D32 — colour is baked per vertex, not resolved by a material.** *(was D17)*
      Classification, ramps and categories already exist on the CPU from C1c;
      reusing them means the 3-D view and the map cannot disagree about what a
      class is coloured, at four bytes a vertex.
    → **result:** 2 new suites (20 + 20 tests); 22/22 ctest green, clean under
    ASan, docs 0 warnings. **27/27 mutations bite.** Capture:
    `artifacts/composer-scene.png`.

    Three real defects the gates found:
    - **Framing by the rectangle's centre loses the near strip at any tilt.**
      A tilted frustum's ground footprint is a trapezoid whose centre lies
      beyond the point being looked at. `setGroundExtent()` now *solves* for
      the placement — with the target on the ground plane the configuration
      scales about it, so one unit-scale probe determines scale and offset
      exactly — and containment holds to single precision at every tilt.
    - **`fitTo()` framed the footprint and then lowered the target into the
      relief**, which puts the eye below the rim of anything concave: a bowl
      was fitted from inside itself. Found by looking at a capture, not by a
      test; now frames the bounding sphere, and has a gate.
    - **Two mechanisms normalised face winding** — a CPU-side normal flip and
      the material's two-sided term — so neither was load-bearing and the
      test only caught losing both. The material's term is the one that is
      required anyway (a camera orbited *beneath* a surface has the light
      behind it regardless of winding), so the CPU flip was removed and the
      real reason pinned by a test.

    Also: the node-entity guard's mutation survives in the release build and
    is a **heap-buffer-overflow under ASan** — without it, node indices are
    read out of the face-connectivity array. The sanitizer build is the gate.
  - **C3b-1 — prismatic cells and peeling. ✅ DONE, verified 2026-08-25.**
    `LayeredMesh` (a horizontal mesh plus per-column interface elevations),
    `MeshLayer::setLayering()`, `setLayeredValues()` and `setVisibleLayers()`,
    and a prism builder in the scene source. The map is untouched: in plan
    view a layered mesh *is* its own horizontal mesh, so it stays one layer
    seen two ways.

    **The conventions are FVQual's, deliberately.** `SigmaSpec` puts
    **interface 0 at the surface** and 1 at the bed; `LayeredMesh::cell()` is
    `column * layerCount + k` and `interfaceSlot()` is
    `column * (layerCount + 1) + k`. Both are the kind of mistake that yields
    a *picture* — an upside-down reservoir, a transposed water column — rather
    than an error, so both are asserted on numbers. `fromCfSigma()` converts
    from the CF form FVQual actually writes (`z = eta + sigma*(depth + eta)`,
    sigma 0 to -1, depth positive down), so the two sign conventions are
    reconciled once, here.

    **Only the outside is built.** Drawing every cell's six faces is correct
    and unaffordable — at the phase's budget three quarters of the triangles
    are where nobody can see them. Interfaces between visible layers are
    interior and skipped; walls on shared edges are clipped against the
    neighbouring column's own visible slab. That clipping is not a nicety:
    sigma layers follow the bed, so adjacent columns rarely line up, and
    dropping a whole wall because its edge is interior punches a hole into
    the mesh wherever the bed steps — which is the normal case.

    Peeling is what the layered view is *for*: a full stack shows only its
    own skin. The caps of whatever range is left are always drawn, so a cut
    is closed rather than hollow.

    Cell values resolve their colour through the classification directly —
    a cell does not belong to a feature, there being `layerCount` of them per
    face — but it is the *same* classification the map and the legend read,
    so a class switched off in the legend disappears from the prisms too.
    → **result:** 14 tests; 24/24 ctest green, clean under ASan, docs 0
    warnings. **15/15 mutations bite.** Capture:
    `artifacts/composer-layered-both.png`.

  - **C3b-2 — layered files, and the perf question answered. ✅ DONE,
    verified 2026-08-25.** The SDK gained `readVerticalCoordinate()` beside
    its UGRID reader — the vertical coordinate is standard CF, so it is
    format knowledge and belongs there — and Composer gained
    `MeshLayer::fromLayeredUGRIDFile()`, `isLayeredUGRIDFile()` and
    `ugridTimeCount()` over it.

    Found by convention, not by FVQual's variable names: the coordinate is
    the variable whose `standard_name` is an `ocean_sigma_coordinate`, and
    its `formula_terms` names the sigma, eta and depth variables. Interfaces
    come from a CF `bounds` attribute when present; failing that, from a
    variable one longer than the layers whose values run 0 to -1 — which is
    what an interface sigma *is*, whatever a writer called it; failing that,
    midpointed from the centres and **flagged as derived**, since that is
    exact only for a uniform distribution.

    **A read/write asymmetry closed on the way**, the same shape as the
    `node_z` one: the reader could read vertical coordinates the SDK's own
    writer could not produce. `NetCDFUGRIDWriter::setVerticalCoordinate()`
    now writes them, which is also what let Composer test the whole chain —
    it links no NetCDF of its own.

    → **result:** SDK 242 tests (the same 4 pre-existing macOS split-HDF5
    failures); Composer 24/24, clean under ASan, docs 0 warnings. **14/14
    mutations bite** (10 SDK, 4 Composer).

    → **the LOD ladder is not built, and should not be.** Profiled at
    501,760 cells (50,176 columns x 10 layers) on this machine:

    | | |
    |---|---|
    | triangles | 418,432 — a **14x reduction** on the naive six-faces-a-cell 6M |
    | GPU buffers | 36.7 MB |
    | geometry build | ~50 ms |
    | **orbit/pan, cached** | **1.78 ms/frame — 561 fps** |
    | peel rebuild | up to 130 ms |

    Camera interaction clears the phase's 60fps budget by ~9x, so the scene
    is **not draw-bound and a tiled-LOD ladder would address nothing that
    was measured**. The cost that is real is the CPU-side rebuild a *peel*
    triggers: dragging a layer slider rebuilds the geometry at ~8 fps. That
    is the lever worth pulling, and it is a different one — reuse across
    peels rather than fewer triangles drawn. `tools/layered_perf.cpp` is the
    probe; re-run it before believing any of these numbers still hold.

  - **C3b-3 — interactive peeling. ✅ DONE, verified 2026-08-25.** A peel at
    500k cells went from **130 ms to ~21 ms — 6.2x** — putting a peel drag at
    roughly 48 fps where it was 8. Three changes, each measured rather than
    assumed:

    - **The edge adjacency is cached.** It is pure topology and cannot change
      with a peel, yet it was being rebuilt every time: a hash insert and a
      lookup for each of 200k edges. It is now built once, in a single pass
      where an edge's second owner records *both* sides, so the geometry pass
      does no hashing at all. This was the bulk of the win.
    - **Colours resolve once per column**, not once per edge per layer.
    - **The bounds are one reduction at the end**, on plain floats.

    Two things measured the opposite of the obvious guess, and are recorded
    because the obvious guess is what a reader will assume:

    - **Reserving from the previous build made it worse** — 42 ms against
      26 ms with no reserve at all. A peel to one layer needs a fraction of
      what the full stack did, so the reserve allocates tens of megabytes it
      never uses and touching those pages costs more than the reallocations
      it saved. Only the caps, whose size is known exactly, are reserved now.
    - **Growing the bounds per emitted piece was fastest but over-determined
      the answer:** caps and walls each cover the box in almost every mesh,
      so neither was load-bearing and a fault in either hid. Replaced with a
      single reduction over the finished vertex array — same speed once done
      on floats rather than through `QVector3D` and `expandTo`'s branch, and
      one mechanism instead of two.

    → **result:** 19 tests; 24/24 ctest green, clean under ASan, docs 0
    warnings. **7/7 mutations bite.**
    → **the 16 ms budget is not met, and the residue is inherent.** Peeling
    to a single layer *exposes* the steps between neighbouring columns, so it
    emits more wall geometry than the full stack does, not less — which is
    correct, and visible in the capture. Going further means not rebuilding
    at all, which needs the wall clipping expressed against the GPU rather
    than the CPU. Not worth it at 48 fps.
  - **C3c — compositing with the 2D layers.** Network lines draped on the
    terrain or extruded; basemap and raster layers as a draped ground plane;
    the map↔scene view hand-off wired to the UI.
    → verify: a draped line follows the surface it is draped on; camera
    round-trip (3D→2D→3D) preserves extent through the UI, not only the
    camera.

    - **C3c-1 — draping and extrusion. ✅ DONE, verified 2026-08-25.** Every
      vector layer now contributes to the scene, laid on whatever terrain the
      stack holds. The shape of it:

      - **`ITerrainSource`** — one layer answers "how high is the ground
        here", in the *map's* CRS, and everything else in the stack is
        composed against it. Hung off `ISceneSource` rather than off
        `MapLayer`, because only something already drawn in the scene can be
        a surface in it. A mesh offers itself when it draws faces and carries
        node elevations, and **declines when it has none** — a sheet at zero
        is not a surface, and offering it would make a flat drape look like a
        considered one.
      - **`SceneContext`** — geometry is built against what the layer is
        being composed with, not from the layer alone. Draping is a property
        of the composition, so the renderer resolves the stack's uppermost
        visible terrain once and hands it down; a layer that went looking for
        its own terrain would be a layer that knows what else is in the stack.
      - **The sampler interpolates the fan the renderer actually draws** —
        from the ring's first corner. A sample taken off any other
        tessellation of the same face floats above or below the surface it
        was supposed to lie on, and only on the cells that are not planar.
      - **Face lookup is a KD-tree over centroids, widened when it misses.**
        The nearest centroid's face contains the point on any mesh whose
        cells are convex and comparable in size; when it does not, a radius
        search over the largest face's own reach is *exact*, unlike guessing
        at a neighbour count. On a graded mesh — one coarse cell beside a
        column of fine ones — the containing cell is the tenth-nearest.
      - **Off the terrain, a drape holds the last elevation it was given**,
        and a leading run before the terrain starts takes the first. Falling
        to zero is indistinguishable from a hole in the data.
      - **`SceneDrape::{Flat, Terrain, Extruded}`** on `FeatureLayer`, with
        Terrain the default, degrading to Flat by itself when the stack holds
        no terrain. Extrusion stands a curtain from the ground to a height and
        caps it with the crest line; height zero degrades to the crest alone,
        because the unit is whatever the map's CRS measures in and there is no
        default worth inventing.
      - Polygons contribute their **rings, not filled surfaces**: filling one
        against terrain is a constrained triangulation, and an outline that
        follows the ground already says what the map cannot. Points
        contribute nothing, on `MeshLayer`'s own precedent.

      **A real bug surfaced, and neither obvious fix worked.** A line lying
      exactly on the surface it was sampled from has *exactly* the surface's
      depth. Looking straight down — which is the camera the map hands over
      to — the default `Less` discards every line fragment and the whole
      network disappears into the ground. Measured: drawn at 89° of
      elevation, completely gone at 90°.

      - **Depth bias is inert for lines.** Metal, D3D11 and Vulkan apply the
        rasterizer's bias to filled primitives only. Measured: pixel-identical
        with and without, in both the visible and the vanished case.
      - **`LessOrEqual` does not cover it either.** The two depths differ by
        how each interpolator rounded, which is not a tie.
      - **A clip-space nudge in the vertex shader does**, scaled by `w` so it
        is constant in normalised depth, applied to line batches alone. It
        keeps the drape honest: the vertices stay on the surface they claim
        to be on, and only the depth written for them moves. Bounded by a test
        that a line genuinely behind a ridge still loses.

      → **result:** 25 tests; 25/25 ctest green, clean under ASan, docs 0
      warnings. **15/15 mutations bite** (`verification/c3c/falsify.sh`).
      → **note:** the two tests that assert what the *renderer* resolved
      count samples taken from a recording terrain, rather than reading
      heights back. A test that reads heights is testing the drape again; the
      layer that was not chosen is the one that was never asked.
      → a **per-feature extrusion height from an attribute** is the obvious
      next knob — pipe depths are the whole reason to extrude a network — and
      is deliberately not built yet, since nothing has asked for it.
      → *(2026-09-19: points-contribute-nothing and polygons-as-rings are
      taken up by Phase U3a/U3c.)*

    - **C3c-2 — the ground plane. ✅ DONE, verified 2026-08-25.** A raster or
      a basemap has no geometry of its own — it is a picture of a place — so
      its 3D form is the ground itself, wearing that picture.

      - **The texture is made by asking the layer to render itself**, through
        exactly the MapTransform the 2D canvas would use. Not a shortcut: it
        makes the ground plane *by construction* the same picture as the map,
        including whatever reprojection, ramp or tile mosaic the layer does
        inside its own `render()`. A tiled basemap in Web Mercator and a
        raster warped from a state plane both come out right in the map's CRS
        without the scene knowing anything about how.
      - **There are no per-vertex texture coordinates.** A ground plane's are
        an affine function of position, so the shader derives them from the
        world position and the extent — four numbers in the uniform block
        instead of two floats on every vertex, and a drape refined against a
        finer terrain needs no re-coordinating.
      - **A second pipeline, not a branch.** A sampler changes the resource
        binding layout, and a layout is what a pipeline is built against.
      - **Normals come from central differences** over the sampled grid, so a
        draped raster is lit by the relief it lies on rather than shaded like
        a flat sheet.
      - **A basemap reports no bounds**, on the same reasoning that stops the
        map framing the planet — which is also what lets the scene's *focus*
        be read straight off `sceneBounds()` with no rule of its own for
        backdrops. A raster shows all of itself; a basemap shows the focus;
        a scene with only backdrops in it gets nothing, because there is
        nothing for a backdrop to be behind.
      - **The coplanar nudge from C3c-1 generalised.** A ground plane draped
        on a terrain is coplanar with it exactly as a draped line is, so the
        nudge became a *step count*: surfaces 0, ground planes 1, lines 2. A
        single shared step would only have moved the conflict up one layer —
        a network drawn over a draped basemap would then tie with it.

      → **result:** 13 tests; 26/26 ctest green, clean under ASan, docs 0
      warnings. **12/12 mutations bite**
      (`verification/c3c/falsify_ground.sh`), and C3c-1's 15/15 still do.
      → **the fixture had to be redesigned to be worth anything.** A bright
      *quadrant* is invariant under transposing the texture axes — the
      north-west stays the north-west when u and v swap — so the first
      version of this suite passed with the axes transposed. Two stripes of
      unequal value, one down the west and one across the north, swap with
      each other instead. That change immediately caught a real transposition
      that had been sitting in the shader.
      → **framing the ground says nothing about how far back to stand.** The
      near plane is derived from the camera's distance, so relief taller than
      whatever distance happened to be set is clipped away. C3c-3's hand-off
      has to set a distance that clears the scene, not only a ground extent.

    - **C3c-3 — the view hand-off. ✅ DONE, verified 2026-08-25.** Switching
      tabs carries the view. The two views already share a layer stack; what
      they did not share is where they are looking, and a user who frames a
      catchment in one and finds the other showing a continent has been given
      two applications rather than two views of one.

      - **Carried on the tab change, not continuously.** Keeping them in step
        live would reframe the hidden view on every pan of the visible one,
        and a tilted camera's ground extent is a bounding box — so each such
        exchange widens what is shown. Once, on arrival, does not drift.
      - **`setVisibleExtent` became exact.** It was fitting with a 5% margin,
        so a 3D→2D→3D round trip zoomed out 5% *every time*. The margin moved
        out to the commands that frame data — `zoomToFullExtent`,
        `zoomToLayer` — where breathing room is a UX choice rather than
        something charged again on every hand-off.
      - **Neither side hands over a view it never chose.** An empty map shows
        its default rectangle around the origin; handing that over would
        count as framing the scene deliberately, and a model loaded a moment
        later would then never be framed at all. Symmetrically, a scene with
        no 3D geometry reports *no* ground extent, so leaving the map tab and
        returning cannot replace a chosen framing with a couple of units of
        nowhere. The second of those was caught by C1b's own regression test.
      - **Framing on first geometry moved from the first paint to the scene
        changing.** Those are the same moment on a machine with a graphics
        device and are not the same moment anywhere else — and "what is this
        view looking at" should have an answer either way. That also made the
        property testable at all.

      → **result:** 9 tests; 27/27 ctest green, clean under ASan, docs 0
      warnings. **10/10 mutations bite**
      (`verification/c3c/falsify_handoff.sh`).
      → **the round trip is exact only looking straight down**, and the suite
      says so rather than papering over it: tilted, the camera sees a
      trapezoid and `groundExtent()` is its bounding rectangle, so a round
      trip necessarily widens. What is checked there is that nothing is
      *lost* — whatever was on screen is still on screen — and that the
      user's orientation is not the map's to change.
      → **the falsifier harness now detects its own stale patterns.** Two
      mutations silently failed to apply after an edit changed the indentation
      they matched, and read as survivors. It now compares each file against
      its backup and reports "NOT APPLIED" instead.
- **C4. Interaction.** Identify/select on cells, nodes, edges in both cameras
  (picking via existing spatial index patterns + nanoflann); attribute table
  panel for any layer.
  → verify: pick tests at known coordinates return the seeded features in
  both 2D and 3D.

  - **C4a — picking in the map. ✅ DONE, verified 2026-08-25.** A click now
    identifies what is under it, and the selection is one selection.

    - **`CentroidIndex`, shared.** "Which of these is at this point" is the
      question a drape asks of a terrain and the question a click asks of a
      layer, and it is the same short, easily-wrong argument both times: try
      the nearest centroid, because on items that are convex and comparable
      in size it is right and costs one descent; widen to the largest item's
      own reach when it is not, because that is *exact* rather than a guess
      at a neighbour count. `MeshLayer`'s terrain index moved onto it, so
      there is one copy of the argument instead of two.
    - **The widened search returns candidates sorted**, so a click between
      two features takes the one it is nearer to.
    - **Selection lives on the layer**, not on whichever view did the
      selecting — the map, the 3D scene and the attribute table are three
      views of one selection rather than three selections. It travels on
      `appearanceChanged`, because every listener does the same thing with
      it, which is to redraw.
    - **Picking on release, not on press**, and only when the view did not
      move: panning and picking share the left button, and a drag that
      happens to start on a feature is a pan.
    - A polygon is hit from inside *or* from within tolerance of its edge: a
      catchment a few pixels across has no inside to click in, and one filling
      the window is most easily hit at its boundary.
    - Tolerance is in **pixels**, converted through the transform. It is a
      property of pointing, not of the data: a tolerance in metres is generous
      on a city and useless on a pipe.

    → **result:** 18 tests; 28/28 ctest green, clean under ASan, docs 0
    warnings. **14/14 mutations bite** (`verification/c4/falsify_pick.sh`).
    → **a point feature's bounding rectangle has zero area, which makes it
    null** — and both `QRectF::isNull()` and `QRectF::united()` would have
    dropped every point layer out of the index silently. The same trap as
    MeshSpatialGrid's; the bounds are accumulated by hand and judged by a
    vertex count.
    → **the nearest-first fast path hid the sort.** The mutation that
    unsorted the widened search survived, because every test reached its
    answer without widening. It takes a decoy whose *centroid* is nearest and
    whose geometry is nowhere near — three sides of a square, clicked at its
    centre.
    → single-selection only; **shift-to-add is not built**, since nothing has
    asked for it. *(Asked 2026-09-19 — Phase U6c.)*

  - **C4b — picking in the 3D view. ✅ DONE, verified 2026-08-25.** A ray
    from the camera through a pixel, turned into a place on the ground, and
    then the question the map already answers.

    - **The ray is built in *world* coordinates, with the model matrix
      undone.** That is what makes vertical exaggeration a display property:
      a pick lands on the same feature at any exaggeration, and the ray still
      corresponds to what is on screen, because the terrain it is tested
      against is in world coordinates too.
    - **Ground first, features second.** The alternative — intersecting the
      ray with every triangle the scene draws — would answer a slightly
      different question and would answer it *differently from the map* on
      exactly the features whose 3D form is not their 2D one. Named limits
      instead: an extruded curtain is picked where it stands, and a peeled
      column of prisms identifies the column rather than the cell.
    - **March, then bisect.** The step is bounded by the terrain's own
      spacing *and* by the ray's length; both are needed, and the second is
      the one that is easy to leave out.
    - **The click tolerance is measured on the ground**, by casting a second
      ray a few pixels away: a pixel covers a different amount of world at
      the near edge of a tilted view than at the far one, and under
      perspective the difference across one screen is easily tenfold.
    - **The one-selection rule moved onto `LayerStackModel::selectOnly`.**
      Two views spelling it separately would be two places for it to be
      spelled differently.

    → **result:** 28 picking tests and 25 camera tests; 28/28 ctest green,
    clean under ASan, docs 0 warnings. **13/13 mutations bite**
    (`verification/c4/falsify_scenepick.sh`).
    → **two mutations survived the first run, and both were the test's
    fault.** Leaving the model matrix in the ray is *invisible* to any test
    that intersects the ground plane — scaling z leaves where a ray crosses
    z = 0 exactly where it was — so it takes a terrain and an exaggeration
    other than 1. And the step bound had no fixture that needed it until one
    was built where the terrain's mean cell is enormous and its relief is
    not: two plains either side of a ten-unit ridge two hundred high, which a
    step sized on spacing alone walks straight over.

  - **C4c — the attribute table panel. ✅ DONE, verified 2026-08-25 — C4
    COMPLETE.** A table over any layer's `IAttributeProvider`, following the
    selection both ways.

    - **A row *is* a feature index.** The same number picking returns and the
      selection speaks in, which is what lets a row and a feature refer to
      each other with no lookup between them. The vertical header shows it.
    - **Values are read through, never copied in.** A results layer's values
      change while it is being watched and nothing signals it; reading
      through means the table cannot be stale. The cost is a virtual call per
      cell, which for the few hundred a table shows is nothing.
    - **The table follows the selection to another layer.** A click that
      lands elsewhere switches the chooser, because a table still describing
      the layer nobody just clicked is describing the wrong thing.
    - **Writing goes through `LayerStackModel::selectOnly`**, so selecting a
      row clears the other layers exactly as a click on the map does — one
      rule, in one place.
    - Only layers that *have* attributes are offered; a basemap has no table,
      and offering an always-empty one is offering a dead end.

    → **result:** 19 tests; 29/29 ctest green, clean under ASan, docs 0
    warnings. **11/11 mutations bite**
    (`verification/c4/falsify_table.sh`).
    → **ASan found a use-after-free that no test would have.** A window tears
    its layer stack down *before* the docks that show it, and a header view
    re-laid-out during that teardown asks a destroyed layer how many features
    it has. Both the model and the panel now watch `QObject::destroyed` and
    let go. This is the second time the ASan gate has earned its place.
    → **the falsifier harness was reading a crash as a survivor.** Removing
    the re-entrancy guard between the two selection directions does not fail
    the suite — it overflows the stack, so no `[  FAILED  ]` line is ever
    printed. Every C3c and C4 script now judges the exit status as well as
    the output, and bounds the run so a hang reports rather than wedges.
    → multi-row selection is not offered: the table holds one at a time,
    matching what a pick can express, because a selection the map cannot show
    is a selection that only half exists.

- **C5. Layer properties, CRS management, and view controls. ✅ DONE,
  verified 2026-08-26** (b0dccda, d2908bf, e3db9c7, 48e84a7, 9e507e4). *(added to
  scope 2026-08-25, after an audit against openswmm.gui found five capabilities
  that the model layer supports but that nothing exposes — or that were never
  planned at all)*

  What the audit found, so this phase is scoped against facts rather than
  impressions:

  | Capability | Model layer | UI |
  | --- | --- | --- |
  | CRS definition + projection on the fly | ✅ C1a/C1b/C1d | ❌ map CRS is **hard-coded** to Web Mercator |
  | Assign / reproject a layer's CRS | ⚠️ read from the file only | ❌ nothing |
  | Drag (rubber-band) zoom | — | ❌ wheel, ±, and zoom-to-extent only |
  | Drape a basemap/raster over a DEM | ✅ C3c-1/C3c-2 | ❌ **always on**, no way to choose |
  | Orthographic ⟷ perspective in 3D | ✅ `Camera::setProjection` | ❌ no caller |
  | Layer properties dialog | ✅ `LayerStyle` | ⚠️ one symbology page, and **only for layers that have a style** |
  | Map scale, coordinates, CRS on the status bar | — | ❌ run progress only |

  Three scope decisions taken with the user, 2026-08-25: the scale widget is
  **editable** (type or pick `1:N` to zoom) rather than a readout, since that is
  the reason a GIS puts scale in the status bar at all; a **coordinate readout**
  is included beside it; and the phase runs **in full, as committed slices**,
  any one of which is a stopping point.

  - **C5a — a properties dialog for every layer. ✅ b0dccda.** openswmm.gui's
    `LayerStyleDialog` is six tabs — Information, Source, Symbology, Labels,
    Rendering, Metadata — and is the target shape. Composer's is a single
    symbology form gated on `canStyle()`, which means a basemap, an image
    overlay or any style-less layer offers **no dialog at all**; that gate was
    right when the dialog only edited symbology and is wrong once it also
    carries source, CRS and rendering. Tabs become conditional on what the
    layer has rather than the dialog being withheld whole.
    → verify: every layer type in the stack — vector, raster, mesh, tile
    basemap, component data item — opens the dialog and shows at least
    Information/Source/Rendering; a layer with no style shows no Symbology tab
    and no empty one; an edit applied in the dialog reaches the map, the 3D
    scene, the layer tree legend and the attribute table without any of them
    knowing about the dialog (D14/§5.1 — one document, several views).

  - **C5b — CRS management. ✅ d2908bf.** Three distinct operations that a single "CRS"
    label usually blurs together, and which openswmm.gui already separates:
    1. **the map/project CRS** — a searchable picker over GDAL/PROJ's EPSG
       registry, replacing `composermainwindow.cpp`'s hard-coded
       `SpatialReference::webMercator()`;
    2. **assigning** a CRS to a layer whose file carries none, or carries the
       wrong one — this rewrites what the layer *claims to be*, it does not
       move a coordinate;
    3. **reprojecting** — moving the coordinates.
    A project-CRS change on a populated stack asks which of (2) and (3) the
    user meant, as `CRSChangeDialog` does: reproject the stored coordinates,
    re-render in the new CRS, or cancel. Projection-on-the-fly itself already
    works — `MapCanvas::publishCrs()` broadcasts to every layer and each
    reprojects (vector/mesh through the cache behind `onMapCrsChanged`, raster
    through GDAL's warped VRT, D25) — so this slice is the UI over machinery
    that is built and tested.
    → verify: two layers in different source CRSs register on top of each
    other in a third; the check is a **known answer**, not "something drew" —
    C1a's falsification pass showed that unreprojected degrees also satisfy
    "something is near the origin". Assigning a CRS to a `.prj`-less file
    moves it onto the basemap; assigning the *wrong* one moves it somewhere
    definite and wrong, which is the assertion that proves assignment is not
    quietly reprojecting.

  - **C5c — map tools, and drag zoom. ✅ 9e507e4.** The canvas hard-codes its
    interactions: left drag pans, wheel zooms about the pointer, release picks.
    Adding a fourth behaviour to that chain is where it stops scaling, so this
    introduces a small tool object (pan, zoom-rectangle, identify) that owns
    press/move/release and a rubber band — openswmm.gui has 22 such tools;
    Composer needs three, and the framework is worth exactly what it costs at
    three. Picking (C4a) becomes the identify tool without changing its
    behaviour.
    → verify: a dragged rectangle frames precisely that rectangle, aspect-
    corrected the way `zoomToFullExtent` is; a drag inside the click slop zooms
    in about the point instead of to a degenerate rect (a zero-area rect
    otherwise divides by zero or frames nothing — and note C4a's lesson that
    `QRectF::isNull()` is **true** for a zero-area rect, so the guard cannot be
    written that way); switching tools mid-drag does not leave a rubber band on
    screen.

  - **C5d — 3D view controls. ✅ e3db9c7.** Three controls over machinery C3 already
    built and verified:
    - **projection toggle.** `Camera::setProjection` matches the two
      projections through the ground extent, so the switch preserves what is on
      screen; it needs a checkable pair on the 3D ribbon tab and nothing else.
    - **vertical exaggeration.** `SceneView::setVerticalExaggeration` reframes
      after the change (an order of magnitude of relief otherwise leaves the
      scene outside the view) and C4b proved a pick lands on the same feature
      at any exaggeration — so the control is a spin box over a solved problem.
    - **draping, in the properties dialog** (C5a's Rendering tab): per-layer
      `SceneDrape::{Flat, Terrain, Extruded}` and extrusion height for feature
      layers, and — the specific ask — the same choice for **basemaps and
      rasters**, so a basemap can be draped over a loaded DEM from the dialog
      rather than from code.

      A closer read found the state is not "missing" but **stuck on**:
      `TileLayer` and `GdalRasterLayer` both call
      `buildGroundPlane(m_ground, context.terrain)` outright
      (`tilelayer.cpp:149`, `gdalrasterlayer.cpp:425`), so a basemap always
      drapes whenever a DEM is in the stack and can never be flat. Hence:

    - **D33 — the drape property is hoisted onto `ISceneSource`.** *(was D26; C1d holds D26)*
      `SceneDrape` and `extrusionHeight` live on `FeatureLayer` today, which is
      why the two surface layers have no say. They move to
      `include/scene/scenesource.h` as concrete accessors over protected
      members: every scene source has to answer "flat, or on the terrain?", and
      that is precisely what the scene-facing interface is for. This keeps the
      property off `MapLayer`, so D15's thin base is preserved — a drape is not
      something the stack or the canvas needs. `FeatureLayer`'s existing
      accessors become forwarders, so no call site churns; the surface layers
      pass `nullptr` as the terrain when `Flat`, and document that `Extruded`
      reads as `Terrain` for something that is already a surface. The dialog
      offers each layer only the modes it supports, so a raster is never
      offered `Extruded`.

      No separate "which layer textures the ground" chooser is needed after
      this — each layer answers for itself, which is one less piece of state
      to keep in step.
    → verify: the toggle leaves the framed extent unchanged across the switch
    (that is the property `setProjection` claims); a basemap set to drape
    follows the DEM's relief and a flat one does not, asserted on sampled
    vertex heights rather than on a screenshot; the drape choice survives a
    map-CRS change, since the ground texture is re-rendered through the layer
    and therefore through its reprojection.

  - **C5e — map scale, coordinates and CRS on the status bar. ✅ 48e84a7.** Composer's
    status bar carries run progress and nothing else; openswmm.gui's carries a
    coordinate readout, an editable map-scale combo and a CRS button
    (`swmmvis.cpp:1968-2018`), and that is the target. `MapStatusBar` is a
    widget rather than inline code in the main window, so it can be driven
    offscreen by a test.
    - **the scale is editable, not a readout** (decided with the user): GIS
      presets plus a validator, where picking or typing `1:N` zooms the map to
      exactly that scale and panning writes the current scale back. Both routes
      go through one parse-and-apply slot, so a preset and a typed value cannot
      disagree.
    - **`MapCanvas` gains `scaleDenominator()` / `setScaleDenominator()` /
      `scaleChanged()`**, mirroring openswmm.gui's `mapcanvas.cpp:454-524`.
      Two properties matter and neither is optional: the denominator is
      **DPI-aware** (from `QScreen::logicalDotsPerInchX()`, not a hard-coded 96,
      which is off by ~2× on Retina — a bug openswmm.gui has already been
      through), and **CRS-aware** (projected → metres per linear unit;
      geographic → `(π/180)·R·|cos(lat)|` at the view centre). `SpatialReference`
      gains `isProjected()` and `linearUnitsToMetres()` over the OGR handle it
      already holds; `isGeographic()` is there.
    - **a coordinate readout** (decided with the user), fed from the canvas's
      mouse moves through `MapTransform::toWorld` — it is what makes the CRS
      indicator mean anything, since it shows the units being reported.
    - **the widgets bind to the map tab.** They go dead on the Composition and
      3D tabs: a perspective camera has no single scale, and a disabled control
      says so better than a number that quietly means nothing.
    → verify: **known answers, not "a number appeared"** — 1 m per pixel at
    96 DPI is 1:3779.5 (`0.0254/96` m per screen pixel), and
    `setScaleDenominator(N)` round-trips. The gate that proves unit-awareness is
    a **foot-based** projected CRS reading ~3.28× a metre-based one over the
    same extent: a scale that ignored units would print the identical number
    for both, which is exactly the vacuous pass to rule out. Geographic at
    latitude 60 halves against the equator.

  - **C5f — verification.** The standard this program has held since C1c:
    offscreen ctest suites per slice, an ASan run over the shell tests, and a
    mutation script per slice under `verification/c5/` in which **every**
    mutation must bite. The harness judges exit status as well as output —
    C4c's re-entrancy mutation crashed rather than failing, and was read as a
    survivor until the scripts were fixed.


  **Result, 2026-08-26.** 32/32 ctest (from 29), clean under ASan, docs 0
  warnings, and every mutation bites in all five scripts under
  `verification/c5/` — 11 + 10 + 10 + 9 + 10.

  **Two deviations from the plan as written, both narrowing it:**
  - **C5b does not ship a CrsChangeDialog.** openswmm.gui offers
    reproject / re-render / cancel because its model *holds* coordinates.
    Composer's layers keep their own and are reprojected as they are drawn,
    so there is no third option to offer: the map CRS simply re-renders, and
    assigning a layer's CRS is confirmed with a message that says plainly it
    changes what the coordinates mean rather than moving them.
  - **C5c ships two tools, not three.** Pan already identifies on a click
    that did not move the view, and FeatureLayer holds one selection at a
    time, so an Identify tool would have duplicated Pan exactly.

  **What the falsification runs found that the suites did not.** Four
  defects and two pieces of dead code, none of which a green run would have
  shown:
  - **`setCrs()` never invalidated the cached projection** — only
    `setMapCrs()` did — so a layer whose declared system was corrected went
    on drawing where the old one had put it. The hook is now
    `onProjectionChanged()` and fires for both ends of the projection.
  - **the surface layers recorded a drape without asking for a redraw**, and
    the scene caches the batches it is handed, so the choice would have sat
    in the layer correct and invisible.
  - **the CRS chooser lost its selection to its own list rebuild**: clear()
    moves the current item to nothing, which blanked the code the reselect
    was about to look for.
  - **the click-slop rule had no test**, so a pan ending over a feature could
    have selected it silently.
  - **two pieces of speculative code deleted rather than tested** — an
    explicit repaint after a style-less apply (the setters already announce
    themselves) and a `cancel()` before a tool swap (replacing the tool
    destroys it and its rubber band). A surviving mutation is not always a
    missing test; sometimes it is code with nothing to say.
  - **one mutation is honestly inert here**: the offscreen platform reports
    96 DPI, so substituting the 96 fallback proves nothing. It is retargeted
    to double the DPI, which proves the value reaches the formula — the part
    that is falsifiable in this environment.

  **Ordering.** C5a first, because C5b's assign/reproject and C5d's drape
  controls both land in its tabs; C5b before C5e, because the status bar's CRS
  button opens C5b's picker; C5c is independent and can run in parallel with
  any of them. None of C5 blocks Phase D.

### Phase D — Model-agnostic results viewer

- **D1. Run browser. ✅ DONE, verified 2026-08-26** (b3fa962).
  `RunSession` (catalog now, artifacts lazily), `RunBrowserModel` (run →
  component → item, every column from the catalog) and a `Runs` dock tabbed
  beside the attribute table. Two runs open at once, since comparing one
  against another is what D4 is.
  → **result:** 9 tests; 33/33 ctest, clean under ASan, docs 0 warnings,
  **11/11 mutations bite** (`verification/d1/falsify_runbrowser.sh`). The
  catalog gate runs against the SDK's own reopen fixture — four artifact
  formats — and compares field by field against a second, independent read.

  **Narrowed deliberately:** browsing only. Making an open run a member of
  the composition document — the `open`-mode blocks `compositionForRun`
  builds — changes what a saved document contains, and belongs in its own
  slice with its own round-trip test. Manifest *discovery* under a workspace
  is likewise not built; one manifest is opened at a time.

  Decisions taken here:
  - **D34 — the catalog and the artifacts are opened at different times.** *(was D27; C2 holds D27)*
    Reading a manifest touches no data file, so a run whose artifacts have
    moved still opens and still says what it expected to find. Opening a
    component is where that is discovered, and it reports rather than
    returning a component with nothing in it — which looks exactly like a
    run that recorded nothing.
  - **D35 — an entry is not identified by component and item.** *(was D28)* The SDK's
    fixture records one item into four artifacts, so that pair is not a key.
    The first version of the catalog test used it as one and compared entries
    against each other; the fixture caught it.

  Two gaps the falsification pass found: nothing covered a run whose
  artifacts had moved, and the close-run test had selected a row whose
  number happened to equal its run's, hiding whether the panel walked up to
  the run at all. One mutation was caught only by exit status, having
  crashed rather than failed.
- **D2. Themed animation.** Bind any recorded variable to its layer's
  classification (graduated/categorical, data-defined); time slider +
  animation clock shared across layers with differing time axes (nearest /
  interpolated per D6's readers); active-theme legend.
  → verify: theming a variable on the layered-mesh fixture produces expected
  class breaks; stepping time updates 2D and 3D views coherently (image tests
  at 3 timestamps).

  **The clock landed first (b10c430).** `DataItemLayer` can be asked for any
  level its item carries, and `TimeController` holds an instant rather than a
  step number, so layers recorded on different axes stay together. What is
  left is split into four slices, in this order:

  - **D2a — a recorded item becomes a layer. ✅ DONE, verified 2026-08-26**
    (61f8be0, after the SDK slice below). An item row offers *Show on Map*,
    which asks `RunSession::item()` and hands the result to
    `DataItemLayer::create` — the same layer a live component's output gets,
    so a recorded run themes, animates and picks exactly like a running one.
    → **result:** 3 tests in `test_runbrowser.cpp` (12 total); 35/35 ctest,
    clean under ASan, docs 0 warnings, **6/6 mutations bite**
    (`verification/d2/falsify_showonmap.sh`). The end-to-end gate runs
    against the SDK's own reopen fixture, so the geometry and the values on
    the map are the ones the SDK wrote.

    Two of those mutations survived their first pass by matching an earlier
    identical line — `if (!DataItemLayer::isSpatial(item))` and
    `m_workspace->setCurrentWidget(m_mapCanvas)` each appear more than once in
    the window — so they mutated code the test never reached and reported the
    tested code as untested. Both had to be anchored on their surrounding
    comments. A mutation reported as surviving is a claim about the mutation
    as much as about the code, and this program has now been caught by that
    twice.
  - **D2b — breaks that hold still across time. ✅ DONE, verified 2026-08-26**
    (97e1d6b). `LayerStyle::rebuild` reclassified from whatever the layer was
    showing *now*, so stepping a graduated layer recomputed its class breaks
    every frame: the same colour meant a different number at every step, and
    the legend beside the map was correct only for the frame it was last
    computed on. `numericValues()` becomes virtual and the data-item layer
    answers with the pooled record — see D36 below for why that, rather than
    a freeze applied afterwards.
    → **result:** 6 new tests in `test_timecontroller.cpp` (12 total); 34/34
    ctest, clean under ASan, docs 0 warnings, **6/6 mutations bite**
    (`verification/d2/falsify_breaks.sh`). The one worth naming is the
    classifier reaching past the layer to the non-virtual read: it produces a
    map that is correct in any single screenshot and wrong across two.

    Two pieces were simplified rather than kept, each after working out that a
    mutation would have nothing to change: pooling a single level is the same
    read as showing it, so the guard is only about static items; and the level
    count cannot fall, so nothing was protecting against it falling.

  - **D2c — time slider and animation clock. ✅ DONE, verified 2026-08-26**
    (this slice). Playback lives on `TimeController` — `play`/`pause`,
    `toFirst`/`toLast`, speed in steps per second, and a cycle flag — because
    the same instant is already driven from more than one place and will be
    driven from a plot cursor in D3. `TimeControlPanel` is a view of it and
    holds no time of its own.
    → **result:** 10 tests in `test_timecontrols.cpp`, the icon suite extended
    over the transport's faces; 35/35 ctest, clean under ASan, docs 0
    warnings, **11/11 mutations bite**
    (`verification/d2/falsify_transport.sh`).

    Placed in the central widget under the workspace tabs rather than in a
    dock: the bottom docks are tabbed over one another, and a clock that can
    be tabbed behind the attribute table is a clock you cannot see while
    reading the values it is stepping through. `test_shell_smoke`'s assertion
    that the workspace *is* the central widget was updated to say what now
    holds — that both live in it.

    Two mutations survived the first pass, and they failed in opposite
    directions:
    - **a missing test.** A speed set while playing never reached the running
      timer, and nothing noticed because every test set the speed before
      pressing play. An animation that can only be slowed down by stopping it
      first cannot be slowed down to look at the moment that needed slowing.
    - **code with nothing to say.** The panel carried an `m_updating` flag so
      a slider moved to follow the clock would not report itself as a scrub.
      But `setCurrent` already refuses an instant it is within `kSameInstant`
      of, so the exchange ends at the clock; the flag was a second copy of a
      rule the clock enforces. Removed, and the mutation retargeted at the
      clock's own check — which is now load-bearing for every view that both
      follows and drives it, and is tested directly.

  - **D2d — active-theme legend. ✅ DONE, verified 2026-08-26** (this slice).
    A verification slice, as expected: the layer tree derives legend rows from
    the style on every read (C1c), and nothing had to be built. What was
    missing was the evidence, so three tests now pin it — the rows name the
    classes they stand for, they do not move while the run plays, and a
    re-theme that changes the class count is announced as *rows* rather than
    as new text in the rows that were there.
    → **result:** 3 tests appended to `test_timecontroller.cpp` (15 total);
    35/35 ctest, **9/9 mutations bite**
    (`verification/d2/falsify_breaks.sh`, extended over the stack model).

    The last of those was nearly a vacuous test: `rowCount` reads the style
    live, so it reports the new count whether or not the model announced the
    change. A view that is not told keeps addressing legend rows that no
    longer exist, which is the failure worth catching, so the test watches
    `rowsInserted` as well as counting.

  **D2 is complete: D2a, D2b, D2c and D2d are all in, and the SDK gap
  below that blocked D2a is closed.**

  - **D36 — a time-aware layer classifies over every level it carries, not
    over the level it is showing.** *(was D29; C2 holds D29)* Stated as the provider's answer rather
    than as a freeze applied afterwards: `numericValues()` is already
    documented as "every numeric value of a field, for computing class
    breaks", and for an item recorded through time every level *is* that
    field's values. Making it virtual and answering with the pooled read
    keeps the decision in the layer that owns the data, so both callers —
    `restyle()` after a step and the properties dialog after an edit — are
    right without either knowing about time.

    Consequences taken deliberately: the pooled read covers **every** level
    rather than a sampled fraction, because breaks computed from a sample can
    exclude a later value, and a value outside the last class is not drawn at
    all (`indexFor` returns -1, D21). openswmm.gui pairs its 20% sampler with
    a clamping `classIndexFor`; sampling without that clamp would make
    features vanish mid-animation. Reading every level costs one pass per
    layer, cached and extended incrementally as a *running* component records
    more — so a live layer's range widens when it should rather than being
    recomputed from scratch each step. If a run ever arrives large enough for
    that pass to hurt, the fix is openswmm.gui's pair — sampler *and* clamp —
    not the sampler alone.

  **SDK gap found while scoping D2a: a reopened run's items carry no
  geometry.** `ResultsModelComponent`'s `RecordedItem` is an `AbstractOutput`
  plus `ITimeSeriesComponentDataItem` and nothing else, and `IResultReader`
  reads values and times only — there is no mesh read anywhere in that path.
  The manifest names the attachment (`mesh: "reaches"`, `location: "node"`)
  and the artifacts are UGRID and GeoPackage, which *do* carry the geometry,
  and the SDK already has `readUGRIDMesh` — it is simply not wired into the
  reopened component. So `DataItemLayer::isSpatial()` is false for every
  recorded item, and no recorded variable can be themed or animated on the
  map at all.

  This is the risk table's "SDK gaps discovered mid-build" row, and the
  discipline there is to fix it upstream rather than read the artifact a
  second time from inside the GUI: geometry belongs to the item, and every
  consumer of a recorded run needs it, not only Composer.

  **✅ CLOSED upstream, 2026-08-26** — HydroCoupleSDK Slice 6
  (`plans/COMPOSITION_IO_AND_RESULTS_PLAN.md` §Slice 6), in two commits:
  `db8a0b1` added `IResultReader::readMesh` — additive and defaulted, with
  the two UGRID formats delegating to the `readUGRIDMesh` that already
  existed and GeoPackage reading its `mesh_nodes` point features — and
  `b4ee032` gave the recorded items the spatial interface, backed by a new
  `MeshDefinition` → `PolyhedralSurface` conversion in the SDK's spatial
  adapters. 12 mutations, all caught; the SDK suite is at its baseline of
  4 known NetCDF-writer failures.

  Two things that came back from doing it upstream rather than in the GUI:
  an item with no recorded mesh is a **different class**, so it does not
  claim the spatial interface and hand back nothing — Composer asks once and
  gets an answer it can act on; and "all four formats" turned out to be three
  that carry geometry plus CSV, which reports *why* it has none rather than
  failing. Composer's message to the user says the same thing.

  **Found while wiring D2a, ✅ FIXED 2026-08-26** (`5cd9735`, with
  `sdk 8cffdce`). Composer drew patches for any entity type other than
  Vertex, so an edge-located variable drew one feature per face while
  carrying one value per edge. Nothing said so — the map drew, the legend
  classified, and the colours meant something other than what they claimed.

  It needed both repos, because the geometry did not survive the crossing
  either. `PolyhedralSurfaceAdapter` rebuilt its mesh view by walking the
  surface's vertices, so a surface with faces and edges reported a view with
  neither; and it cannot be recovered after the fact, since a
  `PolyhedralSurface` records polygons rather than which vertex indices each
  polygon used. The caller that has the connectivity now passes it, and a
  reopened run's item is exactly such a caller — it read the mesh a moment
  earlier and was dropping it one call before it would have been kept.

  → **result:** 4 tests in `test_dataitemlayers.cpp` (21 total) plus 1 in the
  SDK; **5/5 and 15/15 mutations bite**
  (`verification/d2/falsify_meshentity.sh`, `verification/s6/falsify_readmesh.sh`).

  Two things this turned up. **Nothing exercised the surface path at all** —
  the mesh tests in that file cover `MeshLayer`, which reads a UGRID file and
  had always handled all three entities; the *component item* branch beside
  it had no test, which is how the gap survived C2. And the fixture has 4
  nodes, 5 edges and 2 faces deliberately: on a single triangle, nodes and
  edges are both three, and every mutation would have produced a map that
  counted correctly and meant nothing.
- **D3. Plots.** Time-series plot for picked features/cells (Qt Charts, as in
  openswmm.gui); vertical **profile/slice** tools for 3D results: column
  profile at a picked cell (value vs elevation vs time) and arbitrary
  vertical transect slice rendered as a 2D section; export CSV/.dat
  (comparison-plot export patterns).
  → verify: plotted values equal `getValuesInto()` reads of the same
  hyperslabs in unit tests; transect slice on an analytic field matches
  expected section rendering.

  Four slices, in this order:

  - **D3a — the series behind a picked feature. ✅ DONE, verified 2026-08-26**
    (5ae9849). `SeriesPlotPanel` over the stack's selection, one series per
    selected feature; `DataItemLayer::valuesOverTime()` is the transpose of
    the read the map does; `dateTimeFromJulianDay()` is the one conversion
    both the transport readout and the plot axis go through.
    → **result:** 10 tests in `test_seriesplot.cpp`; 36/36 ctest, clean under
    ASan, docs 0 warnings, **10/10 mutations bite**
    (`verification/d3/falsify_seriesplot.sh`). The gate is the phase's own:
    the plotted values are compared against a `getValuesInto()` read of the
    same hyperslab, made independently in the test.

    Three defects the tests found, each of which still drew a chart:

    - **A `QDateTimeAxis` labels in the viewer's zone** and offers no way to
      change it, so the axis was relabelling a run by the reader's offset
      while the transport readout named the recorded instant. Instants now
      reach the axis carrying their UTC fields — x positions nobody reads,
      labels everybody does.
    - **An instant an hour past J2000 comes back as 12:59:59.9999**, so
      truncating the remainder of a Julian day labels a whole plot a minute
      early. It rounds.
    - **A layer took its name from the caption alone**, so an item that was
      named and not captioned produced a layer called nothing — in the tree,
      the legend and this panel's own message about it. The id is the
      fallback before "Data item".

    **Four mutations survived a first pass, and three of them because the
    fixture agreed with the mutation by accident**: an uncaptioned item whose
    axis label is already "Value", a selection whose set iteration is already
    sorted, an instant with no fractional part to truncate. A fixture that
    cannot tell the right answer from the wrong one is the quietest way for a
    suite to be green and empty. The fourth was a bounds check worth only the
    message it gives, which the test now reads.
  - **D3b — export. ✅ DONE, verified 2026-08-26** (8967378). Free functions
    in `results/seriesexport.*` over plain data, so the formats can be checked
    without assembling a chart to read one line; `SeriesPlotPanel` gains an
    Export button over them. CSV holds every series against the *union* of
    their instants, keyed on the epoch millisecond rather than the raw Julian
    day; `.dat` holds one series and fans out.
    → **result:** 8 more tests in `test_seriesplot.cpp` (18 total); 36/36
    ctest, clean under ASan, docs 0 warnings, **18/18 mutations bite**
    (`verification/d3/falsify_seriesplot.sh`). The suite writes real files
    under `tests/fixtures/results/export`, so what it checked can be opened.

    Decisions worth keeping: a gap leaves an **empty field**, never a zero —
    a zero reads as a measurement and this is the absence of one; the export
    is **kept as the chart is built**, not read back off it, because the axis
    carries instants shifted so its labels read in UTC and inverting that is
    a rounding no file should rest on; and the stream's status is checked
    before a file is called written, since a disk that fills halfway leaves
    an export that opens perfectly well and is missing its tail.

    Two survivors on the first pass, and neither was a missing test. One was
    a guard in the panel that said nothing the writers already say — deleted.
    The other was **my own mutation failing to model what it claimed**: it
    could not tell apart two doubles the fixture had made identical, so the
    fixture now perturbs one by a single ulp, which is what two components
    recording "the same" instant actually do. And one assertion turned out to
    depend on the fixture directory starting clean, which a mutation run does
    not leave it — it clears its own file now rather than making a claim
    about someone else's.
  - **D3c — column profile at a picked cell. ✅ DONE, verified 2026-08-26**
    (1258c86). `MeshLayer::columnProfile()` reads one column's cells against
    the elevations the layering puts them at; `ProfilePlotPanel` draws value
    across, elevation up, following the same selection the map and the scene
    already share.
    → **result:** 8 tests added to `test_layeredmesh.cpp` (27 total); 36/36
    ctest, clean under ASan, docs 0 warnings, **11/11 mutations bite**
    (`verification/d3/falsify_profile.sh`) — one of them only by exit status,
    having segfaulted rather than failed, because dropping that guard reads
    past the value array rather than merely drawing the wrong picture.

    Elevation is the axis and not a layer index: a sigma layering packs its
    layers towards the surface, so a profile against layer number flattens
    the gradient it was opened to look at. Values sit at layer **centres**,
    since a cell's value belongs to the layer and not to a boundary it shares
    with the cell above.

    Three mutations survived a first pass, all because the assertion was
    aimed beside what it meant to pin: axis *ranges* are computed separately
    from the points, so swapping the append order still leaves the axes right
    and draws the profile on its side; the chart's own series were never
    counted, so series left attached would accumulate unseen; and the fixture
    layer was named "edges", which the panel puts into every message, so
    "the message says edge" passed on messages that said nothing of the sort.

    **Narrowed deliberately: no time axis.** The plan's line reads "value vs
    elevation vs time", and the elevation half needs a layered `MeshLayer`,
    which is a file layer that does not step. `TimeController` drives
    `DataItemLayer`s only, and a layered file's *geometry* moves with its
    surface, so stepping one means re-reading the vertical coordinate per
    step and not just the values — a change to what `MeshLayer` is, and its
    own slice. What ships profiles the state the layer holds.
  - **D3d — vertical transect slice. ✅ DONE, verified 2026-08-26**
    (`e286940`). Three pieces, MVC as usual. `results/transect.*` is the
    model half and holds no widget: `spansAlongLine()` answers "which cells
    does this line cross, and where along it" over plain rings, and
    `TransectSection` holds the cells with their distances, elevations and
    values. `MeshLayer::transect()` cuts against the layer's **projected**
    geometry, not its stored mesh, so a section cut in one projection and
    read in another describes the same ground. `TransectPanel` paints it —
    filled cells, not a curve — in the colours `colorForCellValue()` gives
    the map, which is why that accessor is now public. `TransectTool` drags
    the line; `MapCanvas` holds it and draws it, because it is a thing the
    user drew on the map and the panel is a second view of it.

    **The slice decides by containment, not by pairing.** A cell's span is
    the interval between crossings whose *middle* it contains. The obvious
    alternative — pair the crossings up, enter with exit — agrees on every
    line that starts outside the mesh and crosses cleanly, which is why the
    concave-cell test had to be rewritten to start the line *inside* a cell
    before it could tell them apart. It also handles a line lying wholly
    inside one large cell, which crosses nothing at all.

    **Peeling does not apply.** The scene peels because a full stack of
    prisms shows only its own skin. A section is already a cut.

    **A tool cancels through `cancel()`, not through `~Tool()`.** Written
    first as a destructor, which is how the rubber band tools end a gesture
    — but their band is a child widget and this preview lives on the canvas.
    Mutation 19 turned the guard on that destructor into an unconditional
    clear and the suite died of **heap corruption two tests later**: the
    destructor runs while the canvas is being torn down. `MapTool::cancel()`
    is now virtual, `setToolKind()` calls it before the swap, and nothing
    reaches into a dying canvas.

    21/21 mutations bite. Two survived the first pass and **both were the
    test's fault**: a fixture where features and faces are 1:1, so reading
    the column by feature index is right by accident (fixed with a mesh
    carrying an unusable face in front, which the layer skips); and a
    mutation that failed to model its own claim — sampling the interval at
    0.1% instead of 50% is still strictly inside, so it proved nothing.

  D3a and D3b act on any time-aware layer, so they need nothing from the 3D
  side; D3c and D3d need a layered fixture and are where the entity question
  D2's mesh-entity fix raised comes back — a profile is only meaningful for
  values on cells or nodes, not on edges.
- **D4. Comparison.** Two-run comparison: side-by-side or difference theming
  for a shared variable on a shared mesh, and multi-run series overlay in
  plots.
  → verify: difference of a run against itself themes to identically zero;
  overlay plot lists both manifests' provenance.

  - **D4a — difference theming. ✅ DONE, verified 2026-08-26** (`6a2a7c2`).
    `DifferenceLayer` is a layer, not a report: it holds its own geometry
    and one attribute, so classification, the legend, the attribute table,
    the 3D scene and the clock all treat it as ordinary. It owns the two
    `DataItemLayer`s it reads rather than borrowing them from the stack, so
    removing the layer a comparison was started from cannot leave it reading
    freed memory. Entry point is *Compare…* on an item row in the run
    browser, enabled only once a second run is open.

    **Refused, not approximated, when the runs are not on the same ground.**
    Compared vertex by vertex to a tolerance taken from the extent. A
    feature-count check alone waves two different meshes of the same size
    straight through, and the map that results is a map of nothing that
    looks exactly like a map of something.

    **Matched by instant, never by level index.** The same rule the clock
    uses. A run with no time axis is a baseline and answers with its only
    slice — the one the map draws, which is its *last* index on every
    non-entity axis, not its first.

    **`ITimeLayer` extracted** from `DataItemLayer` so the clock depends on
    the capability rather than the class. One kind of time-aware layer made
    a `dynamic_cast<DataItemLayer *>` fine; a second one makes it a chain
    every caller has to extend.

    14/14 mutations bite. One survived the first pass as an **equivalent
    mutant over the fixtures available** — reading a static baseline at
    level 0 instead of at "no level" is identical for a 1-D item, and
    differs only when the baseline has a second axis. Rather than wave it
    through, the suite gained a `{layers, geometries}` static stub, where
    the two answers are the first slice and the last.

  - **D4b — multi-run series overlay. ✅ DONE, verified 2026-08-26**
    (`dee38f6`). **Not** "plot every layer that has a selection", which is
    what this entry said and what the stack forbids: `selectOnly()` clears
    every other layer, deliberately, because a table can only show one
    layer's rows. So the overlay is driven the other way — pick a place on
    one run, and the plot draws what *every* run recorded there. That is
    also the gesture anyone comparing two runs would actually make.

    Series carry their layer's name (where a run's provenance already
    lives) only when more than one layer is on the chart; with one run
    "Feature 3" is what was picked and the title already names the layer.
    With several the title goes empty — there is no one run the chart is of.
    The value axis takes a caption only when every layer on it shows the
    same variable.

    **Only runs on the same ground are overlaid.** `sameGeometry()` moves
    out of `DifferenceLayer` to a free function over two `FeatureLayer`s;
    both callers ask it the same question, because feature N of one layer
    is feature N of another only when the two were recorded on one mesh.

    Layers are found through `ITimeLayer`, which is what lets a difference
    layer be plotted at all. `valueAttribute()` joined that interface for
    the axis label. 11/11 mutations.

### Phase E — Meshing & model configuration tools (FVQual-ready)

- **E1. Domain editor.** Draw/import (GDAL) boundary polygons, holes, internal
  constraint lines, point features; snap/vertex editing on the map; persist as
  part of a **mesh project** section in the presentation sidecar.
  → verify: round-trip of a domain with holes + constraints; degenerate
  geometry rejected at edit time (zero-area bbox family of traps).

  - **E1a — the domain and its persistence. ✅ DONE, verified 2026-08-29**
    (`7ac26bd`). `mesh/meshdomain.*` holds boundary, holes, breaklines,
    forced points and `maxEdgeLength`, shaped to the SDK's
    `TriangulationInput` so E2 hands it straight over. Two deliberate
    differences: **breaklines are polylines**, exploded to segments only at
    the SDK boundary (a segment pool cannot tell one line from two that
    touch, and the editor has to); and **rings are `QPolygonF`**, which is
    what the map draws and what E1b will edit without conversion.

    Validation targets what passes *quietly*: zero **area**, not an empty
    bounding box (three collinear points have a box, a vertex count, and no
    ground); and a hole outside the boundary, which is not degenerate — it
    cuts nothing and leaves a mesh that looks whole because it is.
    Orientation is imposed in `toTriangulationInput()`, not demanded of the
    user. **Self-intersection is deliberately not checked** — the
    triangulator already refuses one loudly.

    Persists in the presentation sidecar. Found and fixed on the way: the
    sidecar is written only `if (!presentation.isEmpty())`, so a domain
    drawn before any component existed — the order anyone actually works
    in — would have been saved nowhere.

    16/16 mutations. Three first-pass survivors, all fixture faults, one
    worth keeping: the area test used a **unit square at the origin**, where
    the ring-closing edge contributes exactly zero to the shoelace sum — so
    a version that never closed the ring passed. It now uses a 4×3 rectangle
    at (1,1), where that edge is −3 of the 24. Also: a clockwise-only
    orientation fixture cannot catch a converter that reverses
    *unconditionally*; that needs a correctly-drawn ring that must be left
    alone.

  - **E1b-1 — the domain on the map. ✅ DONE, verified 2026-08-29**
    (`da1f31b`). `MeshDomainModel` holds the one live copy and raises
    `domainChanged()`; `DomainLayer` is a view that re-reads it. The model
    owns no widgets and knows about no views (CLAUDE.md §5.1).

    **Four layers, one per part**, not one layer for the domain.
    `FeatureLayer` reports the kind of the *last feature added* and branches
    its hit-testing on it, so a mixed layer renders perfectly and picks as
    whatever went in last — a bug that would surface only in E1b-3 and look
    like a fault in the dragging. Homogeneous layers also let the breaklines
    be switched off while the boundary is drawn.

    Boundary outlined, never filled (filling hides what the domain was drawn
    over); holes filled, because a hole is an absence and the fill says so.
    An edit raises **both** `appearanceChanged()` and `extentChanged()`; an
    edit that changes nothing raises neither. 12/12 mutations, first pass.

  - **E1b-2 — drawing tools. ✅ DONE, verified 2026-08-29** (`6fa45c2`).
    One `DomainDrawTool` for all four parts — the gesture is identical
    (click to place, **right-click** to finish; a point finishes on its own
    first click). Right-click rather than double-click because the canvas
    already forwards every button and a double-click cannot be told from two
    hurried vertices. The tool writes to the model and nowhere else, so a
    drawn domain and a loaded one reach the screen identically. Abandoning
    discards: three clicks is enough to make a ring, which is exactly why
    switching tools must not finish one.

    `MapCanvas` gained a **sketch** (opposite lifetime to the section line:
    a result stays, a gesture goes) and **`setTool()`**, so a tool acting on
    something the map never heard of is built by whoever owns that thing.
    `setTool()` needed `m_customTool` beside it — **the tests found that**:
    without it `setToolKind()` compares against a stale kind, concludes the
    requested tool is already installed, and leaves the custom one running
    for good.

    **Meshing got its own ribbon tab.** Crowding the Map tab measurably
    changed the canvas aspect ratio, which two `test_viewhandoff` cases
    depend on — and phase E has generation, vertical grids, terrain sampling
    and BCs still to add. Those two tests remain brittle to Map-tab chrome;
    flagged, not refactored.

    Domain layers are created on first use, not at startup: four permanent
    rows in a composition that never meshes anything is clutter, and empty
    layers still join the extent every view frames. 12/12 mutations.
  - **E1b-3 — vertex editing and snapping. ✅ DONE, verified 2026-08-29**
    (`f3ac67c`). `DomainEditTool` over one set of handles: drag a vertex to move it,
    click an edge to put a corner there and drag it in the same motion,
    right-click a vertex to take it out. The gestures every GIS node editor
    uses, which is the reason to use them rather than better ones.

    **The reach is `mesh/domainsnap.*`, shared with the drawing tool**,
    which now snaps its clicks and its rubber preview through it too: a
    breakline that starts half a pixel off the boundary it divides is
    invisible on the map and is a gap the triangulator meshes through. Ten
    **pixels** converted to ground, never a ground distance chosen up front
    — a tolerance in metres is unusable at one zoom and grabs half the map
    at another. `minimumVertices` moved off the drawing tool into
    `meshdomain.*` at the same time, because a removal asks the question a
    finished drawing asks.

    A drag writes each position **through the model**, so what is on the map
    mid-drag is the domain itself; dragging a copy and committing it at the
    end is a second piece of geometry that can disagree with the first. The
    dragged vertex is excluded from its own snap, or every drag pins to
    where it started and small corrections — the commonest edit there is —
    cannot be made. A shape left below the count its part needs is
    **removed entirely** rather than left as a two-cornered hole that looks
    whole.

    Handles are drawn by the **canvas**, not by the layer: they appear when
    the editor is picked up and go when it is put down, while the layer
    draws the same shapes whether anyone is editing them or not.

    32 gates, 26/26 mutations caught. Three claims needed a fixture built
    before they would bite: the nearest vertex must be neither the first nor
    the last within reach, or a search that stops at the first hit passes;
    the click that tests clamping has to be past the **end** of an edge and
    still on its infinite line; and a breakline drawn collinear has a
    phantom closing edge lying on top of its real ones, so "a line is not a
    ring" passes on a tool that closes it. One mutation was caught only by a
    **segfault** — the handles gate indexed a list whose size it had merely
    `EXPECT`ed; asserted now, and the same mutation fails by name.
  - **E1c — GDAL import. ✅ DONE, verified 2026-08-29** (`4700268`). Rings,
    lines and
    points from a vector file — read **from a layer already on the map**,
    not through an importer's own file dialog. The composer can already add
    any OGR dataset, choose its sublayer and reproject it, and the user can
    already select features on it; a second reader would be a second
    reprojection path and a second chance for the two to disagree about
    where the same file is. `FeatureLayer::projectedFeatures()` means an
    imported boundary lands exactly where the same file drawn as a layer
    does, and the user sees the data before committing it. *Mesh ▸ From
    Selection ▾* offers the four parts from one button, because the Mesh tab
    has generation, vertical grids and BCs still to come and E1b-2 measured
    that crowding a tab changes the canvas below it.

    **Which ring is a hole is read off the geometry, not off the order it
    arrived in.** The obvious rule — OGR gives the outer ring first and the
    interior rings after it — is true of a `Polygon` and false of a
    `MultiPolygon`, whose rings `collectOgrGeometry` flattens into one list:
    it would have read the second square of a two-square MultiPolygon as a
    hole in the first, silently, and shapefiles are full of MultiPolygons. A
    ring is a hole when it lies inside an **odd number** of the other rings
    of its own feature. That covers a polygon with holes, a MultiPolygon and
    both combined, and does not consult winding order, which shapefiles and
    GeoJSON disagree about. Depth ≥ 2 is ground standing inside a hole:
    counted and reported, never cut out, because cutting it would remove the
    ground it stands on and a domain has nowhere to put it back.

    Guessing by area is needed only **between features**, where nothing in
    the file says which ring is the domain: the largest outer ring becomes
    the boundary and the rest become holes, compared by **absolute** area so
    a clockwise ring is the same ground walked the other way. Rings arrive
    **opened**, since OGR repeats the closing point and a MeshDomain ring
    does not. The whole import is **one** `setDomain`, so one gesture
    repaints once instead of putting every half-finished state on the map.
    `ringEnclosesArea` moved into `meshdomain.*`: a ring arriving from a
    file asks what a ring being validated asks.

    21 gates over hand-built layers (every coordinate known) and three
    hand-authored GeoJSON fixtures (the reader's real flattening), 18/18
    mutations caught. **Four fixtures agreed with the wrong answer** before
    they were built to discriminate: a MultiPolygon whose *first* ring was
    the larger; nested rings listed outermost-first, so arrival order and
    nesting coincided; rings all wound the same way, so signed area and
    absolute area agreed; and — the one that survived the first falsifier
    run — a collinear ring laid along an **axis**, whose bounding box is
    empty too, so "encloses no area" and "has no bounding box" could not be
    told apart. It is diagonal now. E1a recorded that exact trap and it was
    still walked into from the other side.
- **E2. Unstructured meshing.** Dialogs over SDK `Triangulator` (CDT: size/
  quality controls, region attributes) and `QuadMesher` (quad-dominant);
  async generation with progress + cancel (meshing-OOM hardening lessons:
  guarded results, no unguarded `result()`); preview layer before commit;
  write UGRID via SDK writers.
  → verify: generated meshes are the SDK tools' own outputs (delegation
  test: GUI-produced == direct-API-produced for identical parameters);
  cancel mid-generation leaves no partial state.

  Split as E1b was, because the parts fail differently: the generation has
  to be *right*, and a preview or a progress bar is visible when it is
  wrong.

  - **E2a — generation, behind one delegating call. ✅ DONE, verified
    2026-08-29** (`991ea00`). `mesh/meshgenerator.*`: `generateMesh(domain, options)`
    validates the domain, hands `toTriangulationInput()` to
    `Tools::Triangulator`, optionally passes the result through
    `Tools::QuadMesher`, and reports counts read back off the faces. The
    SDK's tools library had never been linked — the domain header used only
    its input struct — so `HydroCoupleSDK::HydroCoupleTools` joins the link
    line here.

    The edge-length limit is **not** on the options: it has lived on the
    domain since E1a and a second copy would be a second answer. The quad
    threshold is on the options and passed straight through, since it is the
    only control over how quad-dominant the result is.

    The plan's delegation gate is the suite's spine: a mesh built through
    the composer and one built by calling the SDK directly on the same
    domain are compared **node for node and face for face**, not by counts —
    a composer that reordered, welded or nudged anything would still count
    right. The fixture domain carries a boundary, a hole, a breakline and a
    forced point on purpose: a part left out of it is a part nobody checks
    ever reaches the triangulator. 10 gates, 12/12 mutations caught.

    **Two things the plan's E2 line asked for that the SDK could not do**,
    found by reading it rather than by writing against it. One is now fixed
    upstream, the other deliberately is not.

    **Fixed — watching and stopping a run** (sdk `c0165d6`, composer
    `d978439`). Both entry points were synchronous statics with no progress
    callback and no cancellation hook. `Tools::MeshProgress` is now an
    optional callback on overloads of both, and `generateMesh` takes and
    forwards one. Refusing to carry on abandons the run, which reports
    itself **cancelled — not failed**, since telling a user the
    triangulation failed after they pressed Cancel is a lie about their own
    model. A cancelled run carries no mesh at all, not even the
    triangulation finished before an abandoned quad merge.

    E2c must still be honest about what that buys: the fraction is phase
    position, not work remaining, and **the CDT insertions cannot be
    interrupted** — one call into a library with no hook, so a cancel asked
    for during one lands when it returns. What stops promptly is where the
    loops are the SDK's own: densification, extraction, and the whole quad
    merge.

    **Not fixed — region attributes.** `TriangulationInput` has none, and
    CDT has no area-based refinement, so supporting them means the SDK
    growing its own refinement algorithm (Ruppert/Chew). That is a feature
    with its own design, not plumbing E2 needs; the control stays unbuilt
    rather than faked.

    **Not fixed — and a trap for whoever tries.** The tempting cure for the
    self-intersection gap below is to switch CDT from
    `IntersectingConstraintEdges::TryResolve` to `NotAllowed`, which is
    three characters. It would be wrong: boundary rings, hole rings and
    breaklines all enter one edge list, and **crossing breaklines are
    normal** — two channels crossing. A blanket refusal rejects legitimate
    domains. The real question is whether the *boundary ring* is simple.

    **Open, and not E2's to fix: a self-intersecting boundary is meshed in
    silence.** E1a leaves self-intersection unchecked on the stated grounds
    that "the triangulator already refuses one loudly". It does not: given
    `(0,0), (10,0), (2,8), (8,8), (0,4)` — crossing, with real area —
    `isValid()` accepts it and CDT returns three faces and no message. The
    bow-tie that hides this is the symmetric one, whose shoelace area is
    exactly zero and which is therefore refused by the zero-area check, for
    the wrong reason. Needs a segment-intersection pass in the E1a
    validator.

  - **E2b — the preview layer**: the generated mesh drawn on the map before
    it is committed, and gone if it is not.
  - **E2c — the dialog, the worker and the progress it can honestly show.**
  - **E2d — writing the mesh out as UGRID through the SDK writers.**
- **E3. Structured & vertical grids.** `CurvilinearGrid` (boundary-fitted)
  and `SigmaGrid` dialogs; sigma-spec editor (layer count/spacing curves,
  preview of layer interfaces on a terrain transect); `TerrainSampler` (IDW)
  to sample DEM rasters (GDAL) onto mesh vertices/cells.
  → verify: sigma interfaces monotone and matching spec on a synthetic DEM;
  sampled elevations equal direct `TerrainSampler` output.
- **E4. Attributes & boundary conditions.** Attribute assignment on
  cells/edges/vertices (spatial queries: polygon select, expression filter —
  attribute-table query patterns); BC sublayer visualization per type
  (meshbc sublayer port); assignment written to the mesh/UGRID attributes and
  the composition's argument payloads.
  → verify: assignment round-trips through save/load; BC glyphs render per
  type and stay in sync with edits (live-prefs lesson: no silent CPU-path
  omissions).
- **E5. FVQual configuration editors.** Built only against FVQual's argument
  schema once its HydroCouple component exists (E6): kinetics `.rxn` editor
  (text + syntax assist, MSX convention — shared lineage with OpenSWMM's
  engine), forcing editors (sources, structures with rating curves, outlet
  groups with multi-port blending, transfers), open-boundary spec, initial
  conditions. Everything flows through B2's configurator machinery — these
  are schema + custom-widget refinements, not new plumbing.
  → verify: a complete FVQual reservoir test case is configurable start-to-
  finish in the GUI and produces a composition FVQual runs; hydration
  contract test per editor.
- **E6. FVQual component prerequisite (external dependency). ✅ DELIVERED
  upstream** (`FVQual/include/fvqual/component/fvqualcomponent.h`, noted
  2026-09-19): an `IModelComponent` whose arguments expose the mesh
  (`PolyhedralSurfaceArgument`), meteorology (`TimeSeriesArgumentDouble`),
  geometry ints/doubles and a kinetics file (`Argument1D*`). E5 is unblocked
  on the component side; what E5 now waits on is **U2** (typed argument
  editors) and its SDK prerequisite **U2-S** — those arguments derive from
  `AbstractArgument` only and do not implement the typed data-item
  interfaces, so the configurator cannot yet tell a time series from a
  table. E1–E4 continue against SDK fixture components.

### Phase U — UX coherence (added 2026-09-19)

Full text, review findings and gates:
`plans/COMPOSER_UX_COHERENCE_PLAN_2026-09-19.md`. Order agreed with the
user: **U7 → U1 → U6 → U3 → U4 → U5 → U2**, with U2-S (SDK) allowed to run
in parallel from the start.

- **U7. Global preferences.** `PreferencesManager` (table-driven, over
  QSettings, `preferenceChanged(group, key)`), consumers reading live in
  place of the `constexpr` copies (click slop ×3, pick radius ×2, snap
  reach, selection colour, scene background, map CRS, theme, welcome flag,
  recent limit, component search paths — A2's deferred UI), and
  `PreferencesDialog` (categories + scrolled pages, Reset | Apply / Cancel /
  OK, `openAtCategory`). **✅ DONE, verified on macOS and committed
  2026-09-19 (`c98b456`)** — hand-off `plans/U7_HANDOFF_2026-09-19.md`.
  → **result:** 58/58 ctest green (`test_preferences` 11,
  `test_preferencesdialog` 8; the seven new consumer gates in
  `test_welcome`, `test_maptools`, `test_picking`, `test_domainedit` pass by
  name); clean under ASan (the four suites); **15/15 mutations bite**
  (`verification/u7/falsify_preferences.sh`, P1–P5, D1–D4, C1–C6) and the
  welcome falsifier 7/7 with M2b/M4b retargeted; the tree came back
  byte-identical after both. Captures: `artifacts/composer-preferences.png`,
  `-components.png`, `-dark.png`. §4 of the hand-off walked offscreen
  against the real preferences domain with `defaults write` / capture /
  `defaults delete` (`verification/u7/walk/`): welcome off opens on
  Composition; `EPSG:26912` reads in the status bar and `EPSG:999999` falls
  back to 3857; a stored search path fills the palette on start-up with no
  menu action; the domain was byte-identical afterwards. Still owed to the
  eye (needs a live 3D device): re-theming on Apply while the dialog is
  open, red selection in the 3D tab, white 3D background on Apply,
  orthographic + ×3 on relaunch.
  → **note:** the real domain is `org.hydrocouple.HydroCoupleComposer`
  (from the organisation domain), not `com.…` as the hand-off's §4.10 said;
  keys read there as `preferences.<group>.<name>`. Docs build carries 6
  warnings that pre-date U7 (`canvasitems.h:277`, `compositionscene.h:62`
  unknown `@from`; `gdalrasterlayer.h:165`, `tilelayer.h:177` stale
  `@param drape`) — none in U7's files. `test_picking` builds a bare
  QApplication, so its ini lands under `Unknown Organization/`; harmless.
  → findings while building it: `[General]` is Qt's reserved prefix-less
  ini section, so a key stored as `general/x` is *lost across a restart* —
  caught only by a gate that writes ini text by hand, since QSettings serves
  a typed in-process cache; and exclusive radio buttons cannot be cleared
  with `setChecked(false)`, so a revert from Dark to System left Dark lit.
- **U1. Closeable start page.** Close button on the welcome tab alone (at
  the side `SH_TabBar_CloseButtonPosition` asks for); the page is removed,
  not deleted, so *Help ▸ Welcome* and a ribbon *View ▸ Start* face bring
  back the same page with its list intact. Recent-list management — a
  *Clear list* button that goes dead when empty, and an Open / Remove from
  List menu. **Implemented 2026-09-20; verification handed off** —
  `plans/U1_HANDOFF_2026-09-20.md`. Proven off-machine against Qt 6.4: the
  page's half, 4/4 with 6/6 mutations biting; the tab's half (T1–T7) needs
  the Mac. 12 cases in `test_welcome`, 13 mutations in
  `verification/u1/falsify_welcome_tab.sh`, every pattern dry-run against
  the source.
  → **four Qt facts measured before the code was written**, not assumed:
  `removeTab` leaves the page parented to the stack (so the defensive
  reparent was never written); `QTabBar` does **not** delete a tab button
  with its tab, so a fresh button per restore leaks one orphan per cycle —
  four restores left four — hence one button, created once (mutation T6);
  a button can sit on one tab while the others keep none, which
  `setTabsClosable()` cannot do; and Qt already moves to the composition
  when the closed tab was the current one and **stays put when it was
  not**. That last one killed a line in the first draft: an unconditional
  `setCurrentWidget(m_canvas)` is not redundant but wrong — it pulls the
  user off a map they were reading (gate
  `ClosingTheStartPageFromAnotherTabLeavesYouOnIt`, mutation T3).
  → **scope change: the examples section is dropped and folded into F3.**
  The plan said to list the SDK's shipped examples "found through the SDK's
  install prefix". The SDK installs **no** examples (only `schema/`), and
  `examples/serial_coupling/composition.json` names no library and no
  component-info reference — its components are resolved by that example's
  own `main.cpp` through a caller-supplied `ComponentResolver`, by the
  SDK's design. Opened in Composer it draws two *unavailable* boxes, so an
  examples list would be a list of broken links. A real worked example
  needs a demo component in the bundle, which is packaging, and F3 already
  owns "two worked examples".
  → **what replaced it came from the field**: U7's verification found nine
  recent entries in the real plist pointing at deleted temp directories
  from test runs made before the settings redirect existed, and
  `RecentCompositions::clear()`/`forget()` had existed since `fb24baa` with
  no UI at all. Closing the page deliberately does **not** touch the
  start-up preference: "not now" and "not ever" are different questions,
  and the second already has a box on the page and a row in the dialog.
- **U6. Selection across views.** `IComponentLayer` provenance on the
  layers built from a component's data items; `SelectionHub` (owns no
  selection — listens to the canvas, is told about picks, emits what the
  tree and the canvas should do, with a re-entrancy guard); additive
  selection as `SelectionMode{Replace,Add,Toggle}` with Shift adding and
  ⌘/Ctrl toggling on the map click and the rubber band.
  **Implemented 2026-09-20; verification handed off** —
  `plans/U6_HANDOFF_2026-09-20.md`. 11 cases in `test_selectionhub`, 12
  mutations in `verification/u6/falsify_selection.sh`, every pattern
  dry-run against the source.
  → **the off-machine checking got much stronger here.** A syntax harness
  (`plans/setup_syntax_check.sh`) now compiles Composer's translation
  units against the real HydroCouple / SDK / OGC / GDAL / Qt headers with
  no build system, so every file U1 and U6 touch — including the 110 KB
  `composermainwindow.cpp` — passed a C++20 front end before leaving the
  sandbox. And `LayerStackModel` + `FeatureLayer` link against GDAL alone,
  so the whole of U6c was **built and run** off-machine: 5/5 gates, 5/5
  mutations. Its gaps are environment, not code: Qt 6.4 here, so
  `QStyleHints::colorScheme` (6.5+) and `QRhiWidget` (6.7+) are unchecked
  or stubbed.
  → **a guard written, then deleted.** `select()` began by checking
  whether another layer held the selection and replacing rather than
  unioning. Mutation S4 survived — and not for want of a test:
  `selectOnly` guarantees one layer holds a selection at a time, so when
  another holds one this layer's own is empty, and a union with empty *is*
  that replacement. It could not change an outcome. Deleted with the
  reasoning in place; the behaviour is still gated, by what actually
  enforces it.
  → **a null dereference the falsifier found.** Dropping the `!target`
  check — Add or Toggle on a layer with no features, such as a basemap —
  **segfaults**, and nothing covered it until the mutation went looking.
  Now gated, and caught by exit status rather than output: the C4c lesson
  earning its keep a third time.
  → **`DifferenceLayer` deliberately does not carry provenance**: it is
  derived from two runs and belongs to no single component, so it answers
  "no component" rather than half of one.
  → **the attribute table needed nothing**: it is already
  `ExtendedSelection` and already writes whole sets through `selectOnly`,
  so C4c's "the table holds one at a time" is stale against its own code.
- **U3. 2D ⟷ 3D parity.** A point layer contributes solid markers instead
  of nothing (and so does a mesh's node entity); rings fill as surfaces
  when convex, per layer, off by default; the properties dialog gains
  marker size, ring fill and **the peel range, which has had no UI since
  C3b-1**. **Implemented 2026-09-20; verification handed off** —
  `plans/U3_HANDOFF_2026-09-20.md`. 8 cases in `test_scenemarkers`, all
  **built and run off-machine**, with 8/8 mutations biting; 10 mutations
  in `verification/u3/falsify_markers.sh`.
  → **markers are solids, not screen-sized billboards — deliberately.**
  The plan asked for camera-facing quads through a third pipeline with
  its own shader. That is a `.vert`/`.frag` pair and a qsb step that
  cannot be compiled, run or looked at off-machine, so it would arrive
  unverified. An octahedron needs no new pipeline, is correct from every
  angle, is lit by the material already there, and its geometry is
  checkable — and was checked. True billboards remain a shader slice
  worth doing on the machine that can see them.
  → **full U3b is deferred; U3b′ closes the defect that matters.**
  Delegating `DataItemLayer` to a prism/surface builder extracted from
  `MeshLayer` is a refactor of 1,835 lines carrying 27 gates, unrunnable
  from here. Filled **convex** rings do the same job for the case that
  arises, because a mesh face is always convex: a component's
  face-attached output now reads as a surface rather than as wireframe.
  What is not covered is a layered *data item* peeling as prisms, which
  stays a MeshLayer refactor.
  → **the degenerate rectangle, for the fourth time.** Marker size came
  from the extent behind a `box.isEmpty()` guard — and a row of gauges
  along a river is collinear, so its extent has height exactly zero and
  QRectF calls that empty. Every collinear point layer drew nothing. The
  diagonal decides now. D26, C4a and C2 each paid for this same
  rectangle; this one was caught by running the gate rather than by
  reading the code.
  → **an empty batch is not nothing**: the point branch appended its
  batch unconditionally, handing the renderer something to upload and
  draw no triangles from. Appended only when non-empty, as every other
  batch in that function already was.
  → **time in 3D (U3d) is not done**: it needs the RHI render path, which
  no off-machine gate can reach, and belongs with U4.
- **U4. Axis gizmo.** *(implemented 2026-09-20 — `plans/U4_HANDOFF_2026-09-20.md`)*
  `scene/axisgizmo.{h,cpp}`: `buildAxisGizmo()` (three solid arms, E/N/Up,
  red/green/blue), `axisGizmoMatrix()` (rotation only, no model matrix, so
  vertical exaggeration cannot stretch it), `axisGizmoHit()`,
  `axisGizmoView()` (click-to-orient, U4b), plus `axisGizmoRect()`,
  `axisGizmoPoint()` and the corner-name pair. `SceneRenderer` draws it in
  a corner viewport; `SceneView` hit-tests presses against the same
  rectangle. Three preferences under 3D View: visible, size, corner.
  → **the gizmo borrows a real `Camera` rather than re-deriving the eye.**
  The first version wrote the trigonometry out a second time and got two
  gates wrong for two different reasons: at exactly 90° of elevation the
  view direction is parallel to the up vector and `lookAt` collapses to a
  matrix that maps every arm onto the origin. `Camera::setElevation`
  already stops a thousandth of a degree short for precisely that reason,
  so borrowing the camera borrows the guard — and the convention with it.
  → **`copysign` is not a comparison for a component that is zero.** The
  camera-agreement gate compared signs and failed at azimuth 180 and 270,
  where one side produced `+0.0` and the other `-0.0`. The two agree
  *exactly* — every component is the camera's divided by the gizmo's
  orthographic half-width — so the gate asserts that ratio instead.
  → **`QRect::isEmpty()` is the fifth degenerate-rectangle trap.** It is
  true for a width of −16 exactly as for 0, so the gate guarding against a
  negative viewport stayed green while one was produced. Gates on a
  "nothing to draw" rectangle assert a width of *zero*, never emptiness.
  → **an inverted rectangle is more dangerous than an empty one.** Empty
  divides by zero and yields an infinity that every later comparison
  rejects; inverted divides cleanly and yields a plausible point *inside*
  the square, so a press nowhere near the cue answers with an arm.
  → **the corner arithmetic and the Y-flip live outside the widget.**
  `axisGizmoRect`/`axisGizmoPoint` are free functions so the whole click
  path is checkable without a device; what remains in `SceneView` is three
  calls and no arithmetic of its own.
  → **the cue is depth-squeezed, not depth-disabled.** `QRhi` offers no
  mid-pass depth clear, and turning depth off would draw the arms in
  submission order — Up always in front of East. The viewport's depth
  range is compressed into the nearest hundredth of the buffer instead, so
  the cue beats the scene while the arms still occlude each other.
  → **`QRhiViewport` takes OpenGL's bottom-left origin**, and the
  rectangle arrives in the widget's top-left one. Missing that puts the
  cue diagonally opposite the chosen corner, which reads as a broken
  preference rather than a broken renderer; `test_gizmodraw` gates it in
  pixels for all four corners.
  → **D31 is narrower than it was.** `renderSceneToImage()` drives the
  same `SceneRenderer::render()` the widget does, so the cue *is* pixel-
  testable offscreen (`tests/gui/test_gizmodraw.cpp`). And the off-machine
  syntax harness now reaches `scenerenderer.cpp` and `sceneview.cpp` by
  shimming `<rhi/qrhi.h>` onto Qt 6.4's private `qrhi_p.h` — both were
  previously unreachable.
  → **U3d (time in 3D) remains open** and still belongs with the RHI path.
- **U5. Separate 2D/3D controls.** *(implemented 2026-09-20 — `plans/U5_HANDOFF_2026-09-20.md`)*
  New `scene/navigation.{h,cpp}`: `orbitStep()`, `wheelDollyFactor()`,
  `pressShouldPan()`, `namedViewAngles()` and the stored-name pairs for
  `ViewLink` and `PanModifier`. The 3D ribbon tab gains a *Navigate* group
  (Full Extent, Zoom In/Out, Reset View, Top View, Look at Selection, Sync
  from Map); the Map tab gains *Sync from 3D*. Four preferences under 3D
  View: link views, orbit sensitivity, invert wheel, pan modifier.
  → **the menu dispatches and the ribbon does not.** A shortcut means
  "zoom what I see", so the View menu keeps acting on the front view. A
  button on the *3D* strip that moved the map because the map happened to
  be in front would be a button that did nothing visible, so those act on
  the scene unconditionally and bring it forward first.
  → **navigation arithmetic left the widget.** Every decision it can get
  wrong — a drag's sign, an inversion that is not reciprocal, a modifier
  read backwards, a "reset" to the wrong tilt — is now a free function,
  gated and falsified off-machine. Inside a `QRhiWidget` handler none of
  it was reachable (D31).
  → **`pressShouldPan` is not "else if" by accident**: a shift-left press
  that pans must not also start an orbit or a band.
  → **`QRectF::united()` again, sixth time**: the selection's extent goes
  through `expandTo` (D26), or a selection of points frames the last one.
  → **a branch that cannot change an answer was deleted.** The wheel's
  early return for a zero delta was redundant — `pow(x, 0)` is exactly
  1.0 — so it went, and the gate now pins the contract that is actually
  load-bearing: one notch moves by exactly the notch size, Qt's 120
  eighths of a degree and not 8.
  → **Top View and the gizmo's Up arm are gated against each other**, so
  two controls meaning "look down" cannot mean two different things.
- **U2. Typed argument editors as standalone modeless dialogs.** U2-S SDK
  interfaces first; descriptor kinds by `dynamic_cast`; `ArgumentEditorDialog`
  + factory; Table, TimeSeries, Mesh, Geometry, Raster, Quantity,
  DateTime/Duration, LongText, IdTable, Crs.
  → **U2a done 2026-09-20** (`plans/U2A_HANDOFF_2026-09-20.md`). Nine new
  kinds; `describeArgument()` split into gathering and a pure
  `chooseEditorKind(ArgumentFacts)`, which is what finally makes B5a's
  classification chain testable without a loaded component library.
  `ArgumentFacts` carries no id and no caption, so B5a's rule that neither
  may decide an editor is now enforced by the type rather than by
  discipline.
  → **space beats time, and time beats identifiers.** A time-varying mesh
  is a mesh whose values move; a time-varying id table is a table of
  series worth plotting.
  → **Crs has a signal after all**: `validComponentDataItemTypes()` naming
  `ISpatialReferenceSystem`. R3 satisfied, so it is in.
  → **`DateTime` is deliberately left out.** No `DataKind` for it and
  nothing distinguishes a Julian-day scalar from any other `Float64`;
  inventing a signal would violate R3 exactly as it would have for Crs.
  → **widening the enum nearly made arguments vanish.** Nine kinds matched
  no `case` in the configurator's switch, which has no `default:`, so each
  would have produced no widget and been skipped — a component's
  meteorology gone from its form with the build green. The descriptor now
  carries `inlineKind` (the pre-U2a chain, verbatim) beside `kind`, the
  dock draws `inlineKind`, and each dialog that lands moves one kind from
  one to the other.
  → **U2-S done 2026-09-20** in the SDK (`0ce3dbf` on `removing_qt`;
  `HydroCouple/plans/sdk/U2S_HANDOFF_2026-09-20.md`).
  `TimeSeriesArgumentDouble` now answers `ITimeSeriesComponentDataItem`
  and `PolyhedralSurfaceArgument` answers
  `IPolyhedralSurfaceComponentDataItem`, both additively. Nothing is
  stored twice: the Julian days are handed out as a span over the series'
  own vector, and the surface is built from the mesh on demand.
  → **the two caches cannot go stale, by different means.** The series
  validates per slot against its own Julian day, which is exact because
  an `IDateTime` *is* its Julian day — four loaders exist today and a
  fifth would have skipped any invalidation hook. A `MeshDefinition` has
  no cheap identity, so the surface took the other route: all four
  assignments funnel through a new private `adoptMesh()`.
  → **U2b done 2026-09-20** (`plans/U2B_HANDOFF_2026-09-20.md`).
  `ArgumentEditorDialog` (modeless, header + editor + refusal strip +
  Apply/Cancel/OK), `RawArgumentDialog` as the fallback and every typed
  dialog's escape hatch, `createArgumentEditor()` and `hasTypedEditor()`,
  and an *Edit…* on every dock row with one window per argument.
  → **the dialog is handed a committer callback, not a configurator.**
  D8's rule, and also the only reason its gates run without a build: no
  component, no library, no document.
  → **nothing is committed until the user asks.** The difference between
  these windows and the dock's inline editors, which commit on every
  `valueChanged`; a Cancel that left half the typing behind would be a
  lie. A `validate()` hook runs before the committer, so a parse error is
  reported as the dialog's objection rather than the component's.
  → **a gate that only checked the accessor missed an empty window.**
  `payload()` falls back to what the window opened with when the text
  will not parse, and empty text does not parse — so a dialog that
  hydrated nothing answered correctly while showing a blank box. The gate
  now checks the editor's text.
  → **U2c and U2d remain**, in that order.

### Phase F — Hardening, packaging, release

- **F1. Test harness completion.** Offscreen ctest suite covering every
  instantiable dialog/panel (no observer holes); image-baseline management;
  ASan job for the loader; the openswmm.gui build gotchas carried over
  (full-build before trusting verdicts, codesign libomp if OpenMP appears,
  `WA_QuitOnClose` offscreen guard).
- **F2. Packaging.** macdeployqt/windeployqt bundling incl. SDK dylibs +
  GDAL/PROJ data, following openswmm.gui's install/bundle steps; per-OS CI
  artifacts.
- **F3. Docs + examples.** User guide seeded with two worked examples:
  (i) reopen-and-visualize an SDK example run (pure results viewer),
  (ii) mesh-configure-run-visualize FVQual once E5/E6 land. CHANGELOG per
  release (CLAUDE.md 5.2).
  → **also owns the start page's Examples list** (moved here from U1b,
  2026-09-20). It cannot be built before this phase: the SDK installs no
  example compositions — only `share/hydrocouplesdk/schema/` — and its
  `examples/serial_coupling/composition.json` names neither a library nor a
  component-info reference, because each example resolves its components
  through its own `main.cpp` and a caller-supplied `ComponentResolver`.
  Opened in Composer such a document draws *unavailable* boxes, so an
  examples list built on it would be a list of broken links. A worked
  example needs a demo component that ships **in the bundle**, which is F2's
  business; once one exists, the welcome page gains the list and the
  examples above are what it points at.
  → verify (phase): fresh-machine build from README instructions; both
  examples reproduce documented screenshots/outputs.

---

## 6. Milestones

| Milestone | Contents | User-visible capability |
|---|---|---|
| **M0** | A1–A3 | Shell app builds everywhere; loads a component library |
| **M1** ✅ | A4–A5, B1–B4 | Open/import/edit/run compositions — Composer works again, v2-native. **Reached 2026-08-24**: 9 ctest suites green, docs warning-free, GUI assembled and headless run verified. |
| **M2** ✅ | C1–C2, D1–D2 | GIS map with spatial layers; open any run manifest, theme + animate in 2D. **Reached 2026-08-26** (recorded 2026-09-19). |
| **M3** ✅ | C3–C4, D3–D4 | 3D scene; profiles/slices/comparisons — the full results viewer. **Reached 2026-08-26** (recorded 2026-09-19). |
| **M-U** | U7, U1, U6, U3a–c, U4, U5a–b, U2-S, U2a–b, U2c(1–3) | Preferences, closeable welcome, cross-view selection, 2D⟷3D parity, gizmo, per-view controls, typed argument dialogs |
| **M4** | E1–E4 (E5 when U2 lands), F | Meshing + BC tooling; packaged releases |

Sequencing rationale: M1 restores the tool's reason to exist and de-risks the
platform layer first (loader + bridge are novel); the results viewer (M2/M3)
lands before model configuration because it depends on nothing external, while
E5 waits on FVQual's component (E6).

---

## 7. Risks & mitigations

| Risk | Mitigation |
|---|---|
| **dlopen ABI fragility** (C++ across module boundary) | C entry point; interface headers are pure-virtual; registry records/checks toolchain stamps; same-toolchain requirement documented; ASan loader tests (A2) |
| **Component callbacks from worker threads corrupt UI state** | Single crossing point (D4 bridge) with queued marshaling; re-entrancy tests in A3; components never touched from GUI thread while running |
| **3D scope creep** (C3 balloons) | 3D is one camera + geometry-node family over the ported 2D pipeline; Qt Quick 3D explicitly rejected; perf budget fixed (500k cells) with the tiled-LOD ladder inherited from openswmm.gui |
| **Fork drift vs openswmm.gui render core** (D9) | Keep names/structure aligned; log ported fixes both ways; revisit extraction after M3 |
| **FVQual component slips** | E5 isolated behind B2's generic configurator; M4 declared complete-minus-E5 if needed; SDK fixture components exercise all E1–E4 paths |
| **Duplicate netcdf/hdf5 instances in one process** — the SDK's own NetCDF UGRID writer tests fail on macOS this way (4/216, file valid, SDK reader fine, second in-process netcdf/hdf5 copy rejects `nc_open`); Composer links the SDK *and* GDAL, so it can inherit the same clash | Composer's GDAL is pinned to `[geos,jpeg,png,sqlite3]` — no netcdf/hdf5 features — to keep one instance of each; verify at A1 (link) and again at D1 (opening a real manifest); if it bites, the fix is upstream in the SDK/port linkage, not a GUI workaround |
| **SDK gaps discovered mid-build** (e.g. loader/registry, schema holes, reader capabilities) | Fix upstream in SDK, not in-GUI (D6/D7 discipline); this plan's items name the SDK seam they consume so gaps surface as SDK issues early |
| **Legacy `.hcp` corpus small/unknown** | Importer scoped one-way, per-node error reporting; not a gate for M1 sign-off if real projects are unavailable |

---

## 8. Open questions (answer before A1 starts) — **all closed 2026-09-19**

Answers, as the body records them: (1) modernised in place — the v2 tree
lives beside the legacy files in this repository; (2) the stamped
`hydrocouple_component_*_v1` pair, settled at A2, with the unstamped
fallback of D11; (3) Qt Charts, used from D3a on; (4) GDAL is a hard
dependency (C1a onward); (5) Darwin is the exercised preset, Linux/Windows
stay authored-but-unverified until F1.

1. **Repo strategy:** modernize in-place on a `v2` branch of
   HydroCoupleComposer (recommended — keeps history and issues), or a fresh
   repo? The 6 uncommitted 2019-era edits and the `adaptedoutputfactory`
   branch need a decision (commit-as-archive or discard) either way.
2. **Entry-point naming (D3):** finalize the `extern "C"` factory symbol and
   metadata struct with the SDK now, so early test components don't churn —
   and decide whether the loader/registry starts life in the SDK instead of
   Composer.
3. **Qt Charts vs QCustomPlot** for D3 plots — openswmm.gui uses Charts;
   staying aligned is assumed unless there's a reason not to.
4. **GDAL as hard dependency** (assumed yes, for CRS/basemaps/import — matches
   openswmm.gui) vs optional feature.
5. **Windows in scope from M0** (presets say yes) or macOS/Linux first with
   Windows at M4?

---

*Survey sources: HydroCoupleComposer @ `dae8722`; HydroCouple v2.0.0 @
`b94fe7e`; HydroCoupleSDK @ `5a65cd3` (incl. `plans/COMPOSITION_IO_AND_RESULTS_PLAN.md`,
implemented); FVQual @ `527b129`; openswmm.gui @ working tree 2026-08-24.*

---

## 9. OGC connection-management completion (added 2026-09-03, program P6)

From `HydroCouple/plans/sdk/PROVIDER_PIPELINE_PLAN_2026-09-03.md`. The
OgcServiceDialog (one-address WMS→WMTS→WFS→WCS discovery) is the stronger
of the two GUIs' implementations; what openswmm.gui's AddBasemapDialog has
that Composer lacks is the connection-management layer. **License caution:
openswmm.gui is GPL-3.0 — reimplement behaviour, never copy source.**

- **O1 XYZ/custom URL-template mode** + Test Connection (fetch one tile);
  Composer currently offers only 4 hardcoded builtin providers
  (`networktilesource.h`).
- **O2 Saved named connections + machine-bound AES credentials** (QSettings
  store per service kind; AES-256-CBC + PBKDF2 from
  `QSysInfo::machineUniqueId()` — clean-room; user decision 2026-09-03).
- **O3 Per-layer options** (style/format/CRS/tile-matrix-set/interpolation)
  + WMS hierarchy tree (stop flattening `parseWmsCapabilities`' nesting) —
  widen tile-source constructors and `persistentStateForChoice()` /
  `LayerRestorer` in lockstep.
- **O4 HTTP headers** — sequenced: HydroCoupleOgc `HttpClient` accepts a
  header map beside `ServiceCredentials` first (shared-lib change, its own
  verification), then the Composer widget.
- **O5 ArcGIS REST** → derived XYZ connection (optional, last).

Gates extend `test_ogcservicedialog.cpp`; persistentState/LayerRestorer
round-trip for every new connection kind. Also noted: E5/E6 unblock via the
program's P3 (FVQual becomes Composer-loadable with real arguments).

---

## CONNECT additions (2026-09-05/06) — EXECUTED

See HydroCouple/plans/sdk/PROVIDER_PIPELINE_PLAN_2026-09-03.md §CONNECT
for the full record. Composer-side: app icon `22e5abe`; registry kinds/
palette sections/run-pipeline factory resolver `b5e81c2`/`11beae9`/
`5dfe8a6`; canvas adapter nodes end to end `7990dde`/`865b50e`/
`bc3c398`/`1f5ccb3`/`f2701c3`/`7c41b4d` — connection chains are
document content addressed as (endpoint identity, index), spliced
AdapterNodeItems (UserType+5) with sidecar positions, context menus,
the AdapterPickerDialog seams, the AdapterInspector dock, selectable
binding edges, and the 12px port snap. Suite 52/52.

---

## ARGGEN P4 COMPLETE (mirror) — 2026-09-06

B6 delivered: **S4.3 `594df45`** ExecutionPanel (Execution dock tabbed
behind Adapter; QScrollArea mandatory — an unscrolled panel's minimum
width broke the view-handoff framing gate), **S4.4 `908ab7b`**
initializationProgressed forwarding (stage normalized to 1-based, logged
+ status bar). B5a demo-critical subset delivered: **S4.5 `a3174ca`**
binding chip with dangling red badge + unbind (the old text-in-lineedit
rendering was a live mis-affordance — editingFinished would load
"@from x.y" as a file). Full B5a typed-editor sweep stays here as
future scope. Suite 53/53. Details in
`HydroCouple/plans/sdk/PROVIDER_PIPELINE_PLAN_2026-09-03.md` §P4 COMPLETE.

## ARGGEN P6 (OGC completion) — O1/O2/O3a DONE 2026-09-06

- **O1 `dcc96b6`** XYZ/custom URL templates. An address carrying
  {z}/{x}/{y} is RECOGNIZED (not a mode the user picks — this dialog asks
  the server, so it reads the address too) and tested by fetching tile
  0/0/0, whose answer must DECODE as an image: a mistyped template fails
  as a page under a 200. `XyzTileSource : OgcTileSource` (credentials,
  cache, refused-tile memory, reason() — none of which
  NetworkTileSource's built-ins have). Not a ServiceKind; the Choice
  carries a flag. Saved as type "xyz", restored with NO request.
  Falsifier 6/6.
- **O2 `e6c6727`** Named connections + machine-bound credentials.
  `ServiceConnections` over QSettings; PBKDF2-SHA256 100k over
  `QSysInfo::machineUniqueId()` + per-entry salt, AES-256-CBC (OpenSSL —
  Qt ships PBKDF2 but no cipher; GDAL already brings it) + per-entry IV.
  Undecryptable password → EMPTY, address still returned. Divergence
  recorded: openswmm.gui keys by service kind because it makes the user
  classify the URL; this dialog discovers the kind, so one flat store.
  Falsifier 7/7 — it also found the dialog's empty-address check
  redundant with the store's, so the duplicate went.
- **O3a `b661727`** Per-layer style/format, offered from what the CHOSEN
  layer publishes, hidden when there is nothing to choose, and carried
  into the saved recipe AND back out (lockstep). WmsTileSource gained
  `style` (the request struct already had STYLES; the source never set
  it). Falsifier 6/6.

**REMAINING in P6:**
- **O3b — the WMS hierarchy tree.** Blocked on the SHARED LIBRARY:
  HydroCoupleOgc's `WmsLayerInfo` has no depth/parent, so nesting is lost
  at parse time (`readLayer` already appends parents before children in
  draw order and knows its recursion level — adding `depth` is ~4 lines
  plus a gate there). Composer side = replacing the layer QListWidget
  with a tree, which every gate in test_ogcservicedialog drives through
  QListWidget: a wide, mechanical change, its own slice.
- **O4 HTTP headers** — sequenced behind a HydroCoupleOgc `HttpClient`
  change (header map beside ServiceCredentials), as planned.
- **O5 ArcGIS REST** → derived XYZ connection (optional, last). Note that
  O1's XyzTileSource is now the thing it would derive INTO.

Not done deliberately (recorded in `b661727`): CRS and interpolation
pickers. A tiled basemap is Web Mercator by construction and the source
already refuses layers not published in it; interpolation belongs to the
WCS coverage path. Either control would do nothing.

## Shell + canvas round — 2026-09-07

Four user asks; the first two turned out to be one root cause and a
second bug behind it.

- **`4eed36e` Load Directory found nothing.** `scanDirectory` filtered on
  `librarySuffix()` = ".dylib", but a component plugin is a CMake MODULE
  and CMake names a MODULE ".so" on macOS. Every shipped plugin (8 in
  HCC, FVQual's) was invisible. **What hid it: this suite's fixtures are
  SHARED, hence ".dylib" — the loader's gates only ever matched the one
  kind of file the shipped code could see.** Now filters on
  `libraryFilters()` (.dylib/.so/.bundle here). Verified against the real
  directory: 0 → 8. Falsifier 4/4.
- **`35f1ab4` The canvas drew DEFAULT components.** Two causes:
  `ComponentInstances` never applied the document's arguments before
  `initialize()` (so a provider with a configured source still had no
  outputs, and a recorder with a configured path still drew red), and an
  instance was only ever dropped when its component was REMOVED (so
  configuring one changed nothing for the rest of the session).
  Arguments now applied (bindings skipped — Q9), instances re-realised
  when their own block changes by VALUE. Dropped instances are RETIRED
  and released on the event loop: freeing one under a caller that still
  holds the pointer is a crash, which is exactly what the configurator
  suite hit. Falsifier 6/6, one arm deliberately green (every argument
  type also refuses a binding, so the skip is not distinguishable —
  recorded rather than passed off as coverage).
- **`f83dd64` Node auto-width** — the C6 "skippable" item stopped being
  skippable once ports carried real names ("air_temperature" drew as
  "_temperature"). Sized from the widest label PAIR per row, clamped
  [160,260], labels elided per row rather than each given half the box.
- **`9ebdf9f` Auto layout** (Edit ▸ Layout Components). Legacy used
  GraphViz `dot` behind a define only ever set on macOS — elsewhere the
  action degenerated to zoom-to-extent. Reimplemented as a pure
  layered function (rank by longest path, barycentre ordering ×2,
  columns/rows centred) in `canvas/graphlayout.{h,cpp}`: no dependency,
  every platform, gateable without a scene. Cycles are arranged, not
  broken. One undo macro; adapters return to their edges. Falsifier 7/7.
- **`fb24baa` Welcome page + recent documents.** QSettings-backed,
  most-recent-first, capped at 10, missing files marked not dropped;
  File ▸ Open Recent; "show on start up" preference. Clean-room from
  openswmm.gui's behaviour (GPL-3.0). **Its content is scrolled for the
  same reason the execution panel's is** — a QTabWidget's pages share one
  geometry, and a tall page resized the map view behind it. Two existing
  gates updated honestly: shell smoke now CHOOSES its tab (which tab
  opens is a preference now), and the Welcome tab got an icon because the
  icon suite rightly demands one. Falsifier 7/7.

Suite 54 → 56.

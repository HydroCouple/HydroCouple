# HydroCoupleSDK hand-off — interface ABI 4 (contract-consistency round)

**Date:** 2026-09-29
**Driver:** `plans/hydrocouple/INTERFACE_REVIEW_2026-09-29.md` §3–§4, implemented in HydroCouple (see its CHANGELOG, "Contract-consistency round").
**State of this hand-off:** every SDK header, source and test that can be compiled without GDAL/HDF5/netCDF/MPI/Kokkos/Torch was **syntax-checked with `g++ -std=c++20 -fsyntax-only` against the new interface headers** (nlohmann/json and yaml-cpp headers fetched for the check; gtest on the include path for the tests). The SDK was **not built or run** — the toolchain and vcpkg dependencies exist only on the Mac. First action on the Mac: `cmake --build build && ctest --test-dir build --output-on-failure` in HydroCoupleSDK, then the Python bindings' `pytest` in HydroCouple (which was run here: 114 passed).

## What changed in the SDK, by interface change

| Interface change | SDK change |
|---|---|
| `IValueDefinition::valueKind()` replaces `type()` | `ValueDefinition` keeps `type()` as an SDK-only accessor (no longer `override`); gains `valueKind()/setValueKind()`, persisted as `"valueKind"` in `toJson()/fromJson()`; `copy()` for `ValueDefinition`/`Quantity`/`Quality` copies it and recovers the `type_info` via `dynamic_cast` to the SDK class (falls back to `double`). **Default is `ValueKind::Unknown`** — components should call `setValueKind()` on the quantities they create; an adapter that needs the distinction is now entitled to refuse an `Unknown`. |
| `IDimension::role()` | `Dimension::role()/setRole()`. The role is **inferred from the id at construction** (`"time"`→Time, `"layers"`→Layer, `"faces"/"nodes"/"edges"/"identifiers"/"geometries"/…`→Entity, `"x_cells"`→Column, `"y_cells"`→Row, `"z_cells"`→Depth, `"coordinate"`→Component; table in `dimension.cpp`), so every SDK item already answers correctly. `copy()` carries it. |
| `IModelComponent::states()` | `AbstractModelComponent::states()` returns empty by default. **Components with prognostic state should override it** (e.g. the reservoir example's storage; `AtenModelComponent` should return its `differentiableStates()` or a superset). |
| `IArgument::role()` | `AbstractArgument::role()/setRole()`, default `Configuration`. Not persisted in `writeData()` yet — the argument's role is a property of the component's definition, not of a run; decide whether it belongs in the composition spec. |
| `MeshLocation` replaces `MeshDataObjectType`/`NetworkDataObjectType` | Mechanical rename everywhere (`Cell`→`Face`, `Vertex`→`Node`). Entity-dimension ids changed from `"patches"/"vertices"` to `"faces"/"nodes"` in `entityDimensionId()` (layered mesh items, time-polyhedral items). **Persisted result catalogues that stored the old enum integers or dimension ids need remapping** (`resultsmodelcomponent.cpp` reads them back). |
| `entityDimension()` replaces `patchDimension()/edgeDimension()/vertexDimension()`; `vectorBasis()` added | All network/polyhedral items (`LayeredMeshComponentDataItem`, `layeredmeshexchangeitems`, `TimeNetworkComponentDataItem`, `TimeSeriesPolyhedralSurfaceComponentDataItem`, `PolyhedralSurfaceArgument`, `SpatialRecordedItem`) gained `entityDimension()` and `vectorBasis()` (+ `setVectorBasis()` on the templated items; `PolyhedralSurfaceArgument` answers `Cartesian`). `PolyhedralSurfaceArgument` dropped its unused `m_patchDimension/m_edgeDimension`. |
| `ITimeSeriesComponentDataItem` gains `timeKind()/intervalLength()/timeInterpolation()/timeExtrapolation()` | The `TimeSeriesComponentDataItem<T>` mixin holds them (`setTimeKind(kind, intervalDays)`, `setTimeInterpolation()`, `setTimeExtrapolation()`, defaults Unknown) and every concrete item forwards. `TimeSeriesArgumentDouble` holds its own, **defaults Linear/HoldLast because that is what `getInterpolatedValue()` does**, and persists them in `writeData()/assignFromJson()`. `RecordedItem` (results reopen) answers Unknown / `None` / `Refuse` — a recorded series is served exactly at its instants. **`TimeSeriesOutput`/`TemporalInterpolationAdaptedOutput` should declare what they actually do** rather than leave Unknown; `IWorkflowComponent::validate()` is now entitled to reject a temporal connection where both ends say Unknown. |
| `ITimeSpan::endJulianDay()` | `TimeSpan::endJulianDay()`. |
| `IOutput::addConsumer()` is the single wiring entry point | `AbstractOutput::addConsumer()` now checks `canConsume()`, calls `setProvider()`, queues a `Severity::Error` on the owning component (new public `AbstractModelComponent::queueError()`) and throws `std::invalid_argument` on refusal; `removeConsumer()` calls `setProvider(nullptr)`. `ModelInitializer` wires through `addConsumer()` (catching the exception) instead of `setProvider()`+`addConsumer()`. |
| Adapters deregister from their adaptee | `AbstractAdaptedOutput::~AbstractAdaptedOutput()` calls `adaptee()->removeAdaptedOutput(this)`; `AbstractOutput::~AbstractOutput()` calls the new `AbstractAdaptedOutput::detachFromAdaptee()` on every registered SDK adapter so the reverse destruction order (adaptee first) is safe. `ModelInitializer` already destroys `m_adaptedOutputs` before `m_opened`. A caller that takes components via `releaseOpenedComponents()` and destroys them before the initializer is now safe only for SDK adapters. |
| Geometry ownership / `int64_t` widths / `relate(pattern)` | `GeometryAdapter` and friends: `index()` is `int64_t`; `boundary/locateAlong/locateBetween/buffer/convexHull/intersection/unionG/difference/symmetricDifference` return `std::unique_ptr` (still `nullptr` — no geometry engine); `relate()` takes the pattern; `centroid()/pointOnSurface()` return a fresh caller-owned `PointAdapter` (`PolyhedralSurfaceAdapter` keeps the centroid `Point` for that); `boundingPolygons()` returns `unique_ptr` (still null). `pointCount/interiorRingCount/point/interiorRing` are `int64_t`. |
| `IMeshView` connectivity and metrics | `MeshViewAdapter::deriveTopologyAndMetrics()` builds face→edge (CSR), edge→face (left/right, −1 outside) — **appending edges implied by face loops when the `MeshDefinition` carries none** — plus face centroids/areas (shoelace) and edge lengths/normals. `edgeNodes()` may therefore be longer than `MeshDefinition::edgeNodes` for a surface. The UGRID writers can now emit `face_edge_connectivity`/`edge_face_connectivity` from the view. |
| `IPartitionedComponentDataItem::partitionedDimension()` | `PartitionedDataItem` answers 0 (single entity axis). |
| `IExchangeRequest::cancel()` | Every request in `inproctransport.cpp`, `mpitransport.cpp` and `PartitionedDataItem::synchronizeAsync()` answers `test()` (cannot cancel; reports whether already complete). `MpiSendRequest`'s destructor already waited. |
| `IProxyModelComponent::connectToPeer()/disconnectFromPeer()` | Renamed in `ProxyModelComponent` and `test_distributed.cpp`. The old names hid the inherited signal `connect(slot)`. |
| `IExchangeItemChangeEventArgs` removed | `ExchangeItemChangeEventArgs` removed from `abstractexchangeitem.{h,cpp}` (nothing used it). |
| `IUnitDimensions::power()` const | `UnitDimensions::power()` const. |
| `IWorkflowComponentInfo::createComponentInstance()` → `unique_ptr`; `ICloneableModelComponent::clone()` → `unique_ptr` | No SDK implementer of either; nothing to change. |

## Not touched, needs a look on the Mac

- `AtenModelComponent` (Torch): implements `IDifferentiableModelComponent` and checkpointing; should override `states()`. Not compiled here (no Torch).
- `mpitransport.cpp`: `cancel()` inserted mechanically (three requests); not compiled here (no MPI).
- Writers (`hdf5ugridwriter`, `netcdfugridwriter`, `geopackagewriter`): none reference changed symbols, but they *can* now take `IDimension::role()` and `IMeshView::edgeFaces()` instead of positional assumptions — optional follow-up.
- `test_runmanifest.cpp`, `test_executionmodes.cpp`, `test_resultsreopen.cpp`, `test_device.cpp`: only build-time macro / Kokkos errors here, unrelated.
- **Persisted data**: dimension ids `"patches"`→`"faces"`, `"vertices"`→`"nodes"`; enum integers of `MeshDataObjectType` (`Cell=0,Vertex=1,Edge=2,Face=3`) → `MeshLocation` (`Node=0,Edge=1,Face=2,Volume=3`). Anything that stored them must be remapped on read.
- SDK CHANGELOG: an "Unreleased" entry summarizing this table was added at the top.

## Verification checklist

1. `cmake --build build` (SDK) — expect clean; the only new warnings would be from `-Woverloaded-virtual` if enabled, none expected.
2. `ctest` — watch `test_adaptedoutputs` (wiring now goes through `addConsumer()` and throws on refusal), `test_layereddataitem` (bulk `interfaceElevations()`), `test_spatialadapters` (derived edges/faces), `test_resultsreopen` (dimension ids).
3. HydroCouple `python && pip install . && python -m pytest` — 114 expected.
4. Composer: it calls `IProxyModelComponent::connect()` / `MeshDataObjectType` if anywhere — grep before building.

## Built and run off-machine (2026-09-29, Linux, g++ 13) — and four fixes

The working trees above (HydroCouple 26 files, SDK 48) were copied as-is and
**built and run**, not only syntax-checked. The base under them was checked
blob-for-blob against HEAD (HydroCouple fada009, SDK f16a307).

| Check | Result |
|---|---|
| HydroCouple C++ tests | **153 passed** |
| HydroCouple Python (`build_ext --inplace --force`, `pytest tests`) | **186 passed, 3 skipped** (the CUDA rows). The "114" above was a partial environment. |
| HydroCouple `verification/g0g1/falsify.sh` | **24/24 caught** |
| SDK lean build, full suite | 296 passed; only the 4 lean-only `ResultsReopen` failures, which fail identically at f16a307 |
| SDK with NetCDF + HDF5 + GeoPackage | **343/343 passed** (ResultsReopen included) |
| SDK Torch build: `HydroCoupleSDKTorchTests` / `Differentiation*` | **20/20** and **29/29** |
| SDK `verification/g2/falsify.sh` (Torch build) | **46/46 caught** |

The port did not build or pass as delivered. It needed four fixes, all now
written into the SDK working tree (still uncommitted, alongside the port):

1. **`test_device.cpp` did not compile.** The new interface enum
   `HydroCouple::DeviceBackend` collides with the SDK class
   `SDK::Device::DeviceBackend` wherever both namespaces are imported. The
   two uses are now qualified. Other code that does
   `using namespace HydroCouple;` together with `using namespace HydroCouple::SDK::Device;`
   will hit the same ambiguity (the Composer included).
2. **Adapted chains lost their consumer, and teardown crashed**
   (`AdaptedChainTest`: 1 failure and 2 segfaults).
   - Cause: `ModelInitializer` now wires only through `addConsumer()`, but
     `AbstractAdaptedOutput::addConsumer()` was not given the new contract.
     It never called `setProvider()`, so a chain's tail had no consumer.
   - Fix: `addConsumer()` and `removeConsumer()` now behave as
     `AbstractOutput`'s do.
3. **A mid-chain adapter left a dangling adaptee pointer.**
   `~AbstractAdaptedOutput()` did not detach the adapters built on it, so
   the next adapter's destructor called into freed memory. It now detaches
   them, as `~AbstractOutput()` does.
4. **`MeshViewAdapter::edgeCount()` disagreed with `edgeNodes()`.** Once
   face-implied edges are appended, `edgeCount()` still returned the
   definition's count (2 where `edgeNodes()` held 5 edges). `edgeCount()`
   now counts the served edges. `SpatialAdaptersTest.MeshViewServesTheDefinitionsOwnArrays`
   now expects 5, and asserts that the two agree.

Also done, from "Not touched" above:

- `AtenModelComponent::states()` returns `differentiableStates()`.
- The SDK test `Reservoir` overrides `states()`.

Still open:

- **The Composer will not build against ABI 4.**
  `tests/gui/spatialstubs.h` and `test_dataitemlayers.cpp` still use
  `MeshDataObjectType`, `patchDimension()`, `edgeDimension()` and
  `vertexDimension()`.
- MPI and Kokkos builds were not tried.

Mac: run the checklist above, then commit both repos.

## Mac result (2026-09-30, macOS arm64, Apple clang, GNU Make 3.81)

All green; committed, not pushed.

| # | Expected | Mac |
|---|---|---|
| H1 | 153 | 153 |
| H2 | 186 + 3 skipped | 187 + 3 skipped (+1 = untracked generated `include/version.h`) |
| H3 | 24/24 | 24/24, no `SKIPPED GATE` |
| S1 | 358 + 1 skip | 358 + 1 skip |
| S2 | 29 | 29 |
| S3 | 32/32 | 32/32 after a harness fix (first run 29/32, false) |
| S4 | 20 | 20 |
| S5 | no torch in core | none (the literal `grep -i torch` matches the `build-macos-torch` path header) |
| S6 | 46/46 | 46/46, none by build failure |

Fixes made on the Mac:

- `HydroCouple/python/environment.yml`: numpy moved to pip. conda-forge
  numpy's OpenMP OpenBLAS loaded a second `libomp` next to torch's, so
  `import torch` aborted (OMP: Error #15) and H2 died during collection.
- `HydroCoupleSDK/verification/g2/falsify.sh`: `settle()` (1 s) before
  every source write. GNU Make 3.81 compares timestamps to the whole
  second, so a same-second mutant was never compiled and the gate ran the
  previous row's binary. This caused E10, X2 and W1's false SURVIVED; E10
  was caught when applied by hand.

Commits: HydroCouple `98bfbaa` feat(interface): contract-consistency round
(ABI 4); HydroCoupleSDK `2b960aa` feat: port to interface ABI 4. Both were
made through a private index, because both repos hold a stale, empty
`.git/index.lock` from 2026-09-29 13:18–13:19, which was not removed. The
shared index in each repo therefore lists the new files as deleted
(`git diff HEAD` shows `D`; the files are on disk and match HEAD). This
needs `git reset` once the lock is cleared.

Records: `HydroCouple/verification/g0g1/MAC_VERIFICATION_ABI4_2026-09-29.md`
and `HydroCoupleSDK/verification/g2/MAC_VERIFICATION_ABI4_2026-09-29.md`.

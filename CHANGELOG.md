# Changelog

## Unreleased

### Contract-consistency round (breaking; ABI 4) — 2026-09-29

Follows the review in `plans/hydrocouple/INTERFACE_REVIEW_2026-09-29.md`
(§3 robustness, §4 comprehensiveness). Ontology items (§2, §5 items 5 and 13)
are deliberately not part of this round. Every change below is one of:
two normative statements that disagreed, a rule stated in prose but not in
the type, or a principle the standard proclaims that a newer addition
violated.

**Conventions stated once (hydrocouple.h, `\page hc_conventions`).**
Ownership (accessors return non-owning observers; creators return
`std::unique_ptr`), the error channel (`errors()` is normative; lifecycle
methods set `Failed` + queue `Fatal` and *may* throw; `validate()` messages
are queued; every `bool`+`message` failure queues `Error`), the
same-toolchain rule for STL types across the plugin boundary, `status()`
thread-safety, and signal name lookup.

**Lifecycle.** The component transition table now agrees with
`ICheckpointableModelComponent`: `Checkpointing` is entered from `Updated`
*or* `Done` and returns to the state it came from; a successful
`restoreState()` lands in `Updated` (restore is post-`prepare()`, replacing
the prepared state). `finish()` is legal from `Initialized`, `Valid`,
`Invalid`, `Updated`, `Done` and `Failed`, so an `Invalid` composition can be
torn down through the lifecycle. `WaitingForData` has iterative-coupling
semantics (the orchestrator retries; a full cycle of waiting components is a
deadlock). A normative workflow table, `isValidWorkflowStatusTransition()`,
joins the component one.

**Data plane.** `BufferDescriptor` gains `itemSizeBytes` (mandatory for
`Opaque`, so transports and halo exchangers can compute byte extents),
`backend` (`DeviceBackend`: CUDA/HIP/SYCL/LevelZero/OpenCL) and an opaque
`queue` for asynchronous device copies. `DataKind::String` is host-only
metadata that transports and device requests must refuse. Helpers gain
`itemSize()`, `hasValidItemSize()`, `makeOpaqueContiguous()`; the contiguity
and offset arithmetic use the item size.

**Semantics moved onto the types they describe.** `ValueKind` is now
`IValueDefinition::valueKind()` (the `IValueSemantics` side interface is
gone); `TimeKind`/`intervalLength()` are on
`ITimeSeriesComponentDataItem` together with new `timeInterpolation()` /
`timeExtrapolation()` policies (`ITemporalSemantics` is gone, and the enums
now live in `HydroCouple::Temporal`). `IDimension::role()` (`DimensionRole`:
Time, Entity, Layer, Band, Row, Column, Depth, Component, Realization) makes
the prose canonical orderings machine-checkable. `IValueDefinition::type()`
(`std::type_info`) is removed — the element type is the item's `dataKind()`.

**Entities.** One UGRID-named `Spatial::MeshLocation` (Node, Edge, Face,
Volume) replaces `MeshDataObjectType` (which had both `Cell` and `Face`) and
`NetworkDataObjectType`; network, polyhedral-surface and regular-grid items
expose `location()`. `patchDimension()/edgeDimension()/vertexDimension()`
collapse to `entityDimension()`. `SpatialDataType` Vector/Tensor items
declare a `VectorBasis` (Cartesian, EastNorthUp, NormalTangential,
AlongEntity), and the docs of `networkDataType()/meshDataType()` no longer
claim to name an entity. `IPartitionedComponentDataItem::partitionedDimension()`
says which axis is decomposed.

**Components.** `IModelComponent::states()` lists the prognostic items
(what a checkpoint captures; a superset of `differentiableStates()`).
`IArgument::role()` (`ArgumentRole`: Configuration, Parameter,
InitialCondition, Forcing, Geometry). `Capability` gains `LayeredData` and a
vendor-reserved range (`VendorBase`). `IOutput::addConsumer()` is the single
wiring entry point (it calls `IInput::setProvider()`); adapters returned by
`createAdaptedOutput()` are caller-owned and deregister from their adaptee in
their destructor. `ICloneableModelComponent::clone()` and
`IWorkflowComponentInfo::createComponentInstance()` return `std::unique_ptr`.
`IMultiInput` no longer hides `IInput::canConsume()`. Components and
workflows carry using-declarations so both `ISignal` overload sets resolve.

**Time.** `ITimeModelComponent::nextDateTimeJulianDay()`;
`ITimeSpan::endJulianDay()`; `serialDate()` names its epoch (MATLAB
`datenum`); `IOutput::updateValues()` documents what the query specifier
carries (times, geometry, value definition). New
`SpatioTemporal::ITimeLayeredMeshComponentDataItem` and
`ITimeLayeredNetworkComponentDataItem`.

**Space.** `ISpatialReferenceSystem` gains a vertical reference
(`verticalAuthName()/verticalAuthSRID()/verticalSrText()/verticalDistanceUnits()`);
`IVerticalCoordinate` and `ICrossSection` no longer hardcode metres and
refer to it. `IMeshView` gains face→edge and edge→face connectivity (with
boundary markers) and optional metrics (face centres/areas, edge
lengths/normals). `IVerticalCoordinate` gains `columnCount()`,
`geometryEpoch()` and a bulk `interfaceElevations()` span. `ICrossSection`
gains a bulk `evaluate()`. `IGeometry::relate()` takes its DE-9IM pattern;
every geometry-constructing method returns `std::unique_ptr`. `IRasterBand`
reads/writes through `BufferDescriptor`; `RasterDataType` documents its
`DataKind` mapping. Index widths are `int64_t` throughout the spatial header
(`IGeometry::index()`, collections, rings, rasters, grids, layer counts).

**Distribution.** `IExchangeRequest::cancel()`; the destructor always
waits. `IProxyModelComponent::connect()/disconnect()` are renamed
`connectToPeer()/disconnectFromPeer()` — they hid the inherited signal
`connect(slot)/disconnect(slot)`, so a proxy's status signal was
unsubscribable (the Python mirror had already worked around this).
`ITransport` documents payload rules and that collectives are out of scope
until a composition needs them from the standard.

**Removed.** `IExchangeItemChangeEventArgs` (nothing emitted it),
`IUnit::AreaUnits`, `IUnit::DistanceUnitType` (nothing used them),
`IValueSemantics`, `ITemporalSemantics`. `FundamentalUnitDimension` gains
`PlaneAngle`; `IUnitDimensions::power()` is `const`. Stale OpenMI/Qt prose
(`getValue()`, `inputs()` "until validate()", `QImage`) is gone.

**Deferred with reasons.** Transport collectives, boundary-condition roles
on inputs and a conservation report were identified in the review but no
composition in the ecosystem consumes them yet; per the standard's own
no-speculative-abstraction rule they wait for a consumer.

**Tests.** 153 Google Tests (was 146): the reconciled transitions, the
workflow table, opaque byte extents, device addressing defaults, the vendor
range, and two-signal name lookup. 114 pytest (was 98).

**SDK.** HydroCoupleSDK is updated in lockstep (compile-checked against the
new headers; see its CHANGELOG).

### `IWorkflowComponent` advanced for orchestrated execution (breaking)

`validate()` and `prepare()` become explicit lifecycle phases mirroring the
component lifecycle, so a workflow can drive composition-wide validation and
preparation before any update runs. Cooperative `requestStop()`,
`requestPause()`, and `resume()` are honored at synchronization points —
consistent global moments, which is what `ICheckpointableModelComponent`
needs. `errors()` gives the workflow the same diagnostic queue the model
components have, and `addModelComponent()` reports rejection rather than
failing silently.

`WorkflowStatus` accordingly gains `Validating`, `Validated`, `Preparing`,
`Prepared`, and `Paused`. **These are inserted in lifecycle order, not
appended**, so the numeric values of `Updating` and everything after it
shift: `Updating` 3 → 7, `Updated` 4 → 8, `Done` 5 → 10, `Finishing` 6 → 11,
`Finished` 7 → 12, `Failed` 8 → 13. Anything that persisted or transmitted
the old integers must be remapped.

### Testing and CI

- **The Python suite now runs in CI** (`python_bindings` job). It never had,
  which is why `test_enum_parity.py` — written precisely to catch bindings
  drifting from the headers — did not catch the `WorkflowStatus`
  renumbering. The job builds the Cython extension from source rather than
  assuming one: 38 of the 100 tests drive the compiled layer and pass
  vacuously against a stale `.so`. Build plus suite is roughly 30 seconds.
- `test_enum_parity.py` additionally checks the `.pxd` declarations against
  the headers. This is an in-order-subset check, not equality — Cython
  declarations are legitimately partial (`_spatial.pxd` names 5 of
  `GeometryType`'s 72 members) — so it catches a member that no longer
  exists in the header or one reordered against it, neither of which the
  compiler reports while the member goes unreferenced.

### Fixes

- The `WorkflowStatus` renumbering above was not propagated when it landed.
  `tests/test_hydrocouple.cpp` still asserted the old values (this is the
  CI failure on Linux and macOS), and — more seriously — `hydrocouple/core.py`
  still declared the old nine-member enum. Since `IWorkflowComponent.status`
  converts the C++ value through `WorkflowStatus(<int>…)`, a C++ `Validating`
  (3) would have arrived in Python as `Updating` (3) — silent misreporting
  rather than an error, with `Paused` and everything above it raising
  `ValueError`. `_hydrocouple/_core.pxd` was stale too, but harmlessly:
  Cython binds enum members to C++ enumerators by name, so the header
  supplies the values regardless. It is brought up to date as documentation.
- The vcpkg pin (`2025.02.14`) can no longer build on Windows. Old vcpkg
  releases do not stay frozen: `vcpkg_acquire_msys.cmake` pins exact MSYS2
  package versions, MSYS2 mirrors carry only current packages, and the
  pinned `msys2-runtime-3.5.4-2` now 404s on every mirror — so any port
  calling `vcpkg_fixup_pkgconfig` (gtest among them) fails. Both workflows
  now pin `2026.07.29`, matching the other HydroCouple projects.

## 2.0.0-alpha.1 — 2026-08-22

Breaking modernization of the interface standard for HPC, GPU, and cloud execution. Header-only; C++20; MIT. See `docs/INTERFACE_REVIEW.md` for the full rationale.

Two design principles govern the release: the interface headers contain **no executable code** (pure virtual declarations, enums, and plain aggregates only — the few genuinely shared conveniences live in the explicitly non-normative `hydrocouplehelpers.h`, which may be ignored entirely), and **no speculative abstraction** (every interface earns its place against a concrete consumer; unused vocabulary was cut).

### Data plane (breaking)

The variant convention is removed from the standard entirely — `hydrocouple_variant` no longer exists. `IComponentDataItem` exposes a typed bulk API: `getValuesInto()`/`setValuesFrom()` over `BufferDescriptor` hyperslabs (DLPack-style dtype/shape/strides/memory-space plain-aggregate descriptors, `int64_t` indexing). All per-index-kind variant overload sets on the temporal, id-based, spatial, and spatiotemporal data items are deleted; each specialization instead documents a canonical dimension ordering (time outermost, then entity). Former variant metadata accessors became native types: `missingValue()`/`defaultValue()`/`minValue()`/`maxValue()` return `double`, `IQuality::categories()` returns `std::vector<std::string>` (data values are category indexes), `clone()` optional arguments are string-encoded. New core vocabulary: `DataKind`, `MemorySpace`, `BufferDescriptor`, `Capability`, `ErrorEntry`.

### Distribution (new header `hydrocoupledistributed.h`)

Transport-neutral distributed execution: `IExchangeRequest` (async completion handle), `ITransport` (tagged, endpoint-addressed, sync + async send/receive), `IDistributedModelComponent`, an expanded `IProxyModelComponent` (connection lifecycle, ping/timeout, normative failure semantics), and `IPartitionedComponentDataItem` — local virtual (ghost/halo) entity representation with owned/virtual global indexes, owner ranks, epoch-stamped `synchronizeAsync()`. Asynchrony lives only in this layer, where it is load-bearing; the core lifecycle remains synchronous. The MPI-specific methods (`mpi*`) and GPU bookkeeping are removed from `IModelComponent`.

### Core lifecycle and contracts

`ComponentStatus` gains `Checkpointing`; the legal transitions are encoded in `Helpers::isValidComponentStatusTransition()`. New pure-virtual contracts: `capabilities()` query and `errors()` diagnostic queue (the failure channel that crosses process/ABI boundaries), plus `ICheckpointableModelComponent`. UI methods moved off `IModelComponent`/`IComponentDataItem` into `IUIProvider`; licensing moved off `IComponentInfo` into `ILicensedComponent`.

### Spatial and temporal

New `IMeshView`: UGRID-congruent structure-of-arrays access (coordinate spans, CSR `faceNodeOffsets`/`faceNodes`, `edgeNodes`) on `INetwork` and `IPolyhedralSurface`. Regular grids gain bulk `nodeXs()/nodeYs()/nodeZs()` and `activeCells()` spans. Entity counts and indexes widened to `int64_t`. `ITimeSeriesComponentDataItem` gains bulk `times()`; `IDateTime` documents the CF calendar convention. `IArgument` gains `YAML` input type and a symmetric `serialize()` making arguments the normative serialization unit (with external binary payload references for large data).

### Fixes

Erroneous doxygen corrected throughout: the MIT license blurb no longer cites the Free Software Foundation; the duplicated `Failed` status description is replaced; the `getValues` overload documented as "Sets..." is gone with the overload set; `fileFilters()` no longer references Qt; stale duplicate forward declarations removed; copyright years unified to 2014–2026.

### Tests

Google Test suite (140 tests): data-plane types (`DataKind`, `BufferDescriptor` contiguity/offsets/broadcast via the helpers), lifecycle transition table, abstractness/virtual-destructor/inheritance checks for every interface including the new distributed set, and a functional reference `IComponentDataItem` implementation exercising hyperslab correctness (interior slabs, strided/pitched destinations, bounds/kind/rank rejection). Build with `-DHYDROCOUPLE_BUILD_TESTS=ON`.

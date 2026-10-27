# Changelog

## Unreleased

## 2.0.0-alpha.2 — 2026-10-02

Second pre-release of the 2.0 interface, and the release that fixes ABI 4.
It carries the contract-consistency round, the adapter members that let a
derivative cross a stateful adapter, the lifecycle correction, the
component-loading convention, and a Python mirror held to the C++ headers by
tests. The ontology items of the 2026-09-29 review (standard names, concept
URIs, unit symbols, a generated vocabulary) are deliberately not in it.

### Stateful adapters (inside ABI 4)

An adapter whose values depend on its previous refreshes -- under-relaxation,
interpolation over a recorded history -- was invisible as such: nothing
said it carried state, so an orchestrator returning a composition to an
earlier step (reverse-mode replay, a training loop's rewind, a restart)
could not know it had to return the adapter too, and the differentiation
contract was "stateless adapters only". Three additions close that, in the
same unreleased ABI 4 so there is one breaking change, not two:

- **`IAdaptedOutput::states()`** (pure virtual): the items carrying state
  from one refresh to the next, empty for the usual stateless adapter --
  the adapter's counterpart of `IModelComponent::states()`. An item may be
  the adapter itself, when its own previous values are the state.
  **Every `IAdaptedOutput` implementation must add it.**
- **`IDifferentiableAdaptedOutput::differentiableStates()`**: the state the
  derivative follows, a subset of `states()`, in roles `StateBefore` /
  `StateAfter` exactly as for a component; the "stateless adapters only"
  paragraph is replaced. A state item keeps its shape across a refresh.
- **`ICheckpointableAdaptedOutput`**: `saveState()` / `restoreState()` /
  `releaseState()` with the component interface's semantics; adapters have
  no status, so all three are legal after `initialize()`. A restore is not a
  refresh: a differentiable adapter has nothing to differentiate until its
  next one.
- Two conventions made explicit: adapters have no capability set, so their
  optional interfaces are discovered by casting the adapter (`Capability`);
  and an adapter, having no error queue, reports `bool` + `message`
  failures through the message alone (the error-channel convention).
- Python mirror: `IAdaptedOutput.states`, `IDifferentiableAdaptedOutput.
  differentiable_states()`, the new `ICheckpointableAdaptedOutput` ABC; the
  adapter wrapper gains `states` and, when the C++ adapter implements them,
  `differentiable_arguments` / `differentiable_states` / `vjp` / `jvp` and
  `save_state` / `restore_state` / `release_state`. Checkpoint tokens now
  cross the bindings losslessly (component and adapter wrappers alike): a
  token that is not UTF-8 comes back as a `str` that `restore_state()` and
  `release_state()` turn back into the same bytes.
- Tests: member signatures and traits of the three additions (C++); the
  parity test covers the new ABC; a native stateful fixture adapter drives
  every new wrapper member.

The SDK implements them (HydroCoupleSDK 2.0.0-alpha.2): the relaxation
adapter becomes differentiable and checkpointable, and the reverse engine
differentiates through it.

### Lifecycle: `Failed` is reachable from every status but `Finished`

`isValidComponentStatusTransition()` and `isValidWorkflowStatusTransition()`
admitted `Failed` only from the in-flight statuses. That contradicted
`IProxyModelComponent`, whose peer can die while the proxy rests at
`Updated` or `Initialized`, and every orchestrator that fails a resting
component on a partner's behalf. A component or workflow may now enter
`Failed` from any status except `Finished` (`Failed` → `Failed` is not a
transition). The SDK now enforces these tables, which is how the
contradiction surfaced. New tests pin the rule for both tables.

### Python mirror

- **Transition tables were stale since ABI 2.** `helpers.py`'s
  `is_valid_status_transition()` still encoded the ABI-2 component table, and
  there was no workflow table at all. Both are now the ABI-4 tables
  (`is_valid_workflow_status_transition()` is new), and
  `tests/test_transition_parity.py` compiles the C++ helpers and compares
  every (from, to) pair, so the two cannot drift again.
- **Vertical structure.** `IVerticalCoordinate`, `ILayering`,
  `ICrossSection`, `ILayeredMeshComponentDataItem`,
  `ILayeredNetworkComponentDataItem` and the `VerticalCoordinateKind` /
  `CrossSectionKind` enums in `hydrocouple.spatial`;
  `ITimeLayeredMeshComponentDataItem` / `ITimeLayeredNetworkComponentDataItem`
  in `hydrocouple.spatiotemporal`. `interface_elevations` is shaped
  `(column_count, layer_count + 1)`; `ICrossSection.evaluate()` keeps the C++
  out-parameter form (each output `None` or an array filled in place);
  `stations()` returns `(stations, elevations)`.
- **Reading them off C++ components.** Cython wrappers for all of the above,
  and `_hydrocouple._spatial.as_layered(item)` /
  `_hydrocouple._spatiotemporal.as_time_layered(item)`, which take what the
  core bindings hand out for a component's items (a `CppOutputWrapper`,
  `CppInputWrapper` or `CppComponentDataItemWrapper`) and cross-cast to the
  layered interfaces. Interface elevations come back as a read-only NumPy
  view of the producer's own profile; `evaluate()` lends the caller's arrays
  to C++ as spans and releases the GIL, refusing an output it could only
  fill by copying. Argument, input and output wrappers gained `data_item`.
- Tests: the ABCs are checked member-for-member against
  `hydrocouplespatial.h`; native C++ fixtures (a sigma coordinate whose
  surface the test moves, a surveyed trapezoid and an analytic rectangle
  checked against closed forms, a layered network, a time-layered mesh) are
  driven through the bindings; enum parity covers the two new enums.

### Python bindings: every wrapper implements the ABC it is registered as

The Cython wrappers are registered with `ABC.register()` as the interfaces
they wrap, so `isinstance(output, IOutput)` holds and code written against
the ABCs accepts them. Registration checks nothing, and an audit found every
one of the 47 registered wrappers short of its ABC: the time-series wrapper
had no `time_kind` or `time_interpolation`, the workflow wrapper none of
`validate`/`prepare`/`request_stop`/`request_pause`/`resume`/`errors`, the
spatial wrappers lacked identity, navigation and raster I/O, and no wrapper
had the `connect`/`disconnect` the ABCs declare. Each failed at the first
attribute access, far from the cause.

- **Tests that hold the mirror to the standard.**
  `tests/test_wrapper_conformance.py` (71) checks each of the 70
  registrations: every abstract member of the ABC present, and a property
  where the ABC declares a property. `tests/test_abc_parity.py` (94) compares
  every ABC member for member with the pure virtuals its C++ class declares,
  with the deliberate differences listed and explained.
  `tests/test_wrapper_behaviour.py` (34) drives the wrappers against native
  fixtures (`include/binding_test_fixtures.h`, `_testing.BindingFixture`).
  `tests/test_examples.py` (3) runs the examples as scripts.
- **The `IWorkflowComponent` ABC** gains `validate`, `prepare`,
  `request_stop`, `request_pause`, `resume` and `errors`, which the C++
  interface gained in ABI 4 and the mirror missed. The parity test found it.
- **Wrappers restructured on shared bases.** `CppPropertyChangedWrapper` →
  `CppDescriptionWrapper` → `CppIdentityWrapper` → `CppComponentDataItemWrapper`
  carry signals, caption/description, id and the data plane once; each
  subclass binds its own typed pointer by `dynamic_cast`
  (`include/interface_casts.h`), since the interfaces use virtual bases.
  New wrappers cover the remaining interfaces: quantity/quality, multi-input,
  adapted output and its factory, component and workflow info, time-model
  component, time-ID-based items, surfaces, geometry collections and multi-
  geometries.
- **Views.** `_spatial.as_spatial`, `_temporal.as_time_series`,
  `_temporal.as_time_model_component` and `_spatiotemporal.as_spatiotemporal`
  return the most specific wrapper the C++ object implements, or `None`,
  beside the existing `as_layered`/`as_time_layered`.
- **Equality, ownership, signals.** Two wrappers are equal when they wrap
  the same C++ object. A child wrapper keeps its parent alive; an object C++
  creates for the caller (a geometry operation's result, a component
  instance, an adapted output) is owned by its wrapper and destroyed with it.
  `connect(slot)` reaches the signal the ABC documents and passes the
  event-arguments object it promises; connecting a slot twice connects it
  once. One `_SlotHandle` replaces the six per-signal handle classes.
- **C++ exceptions** from mutators (`add_consumer`, `set_provider`,
  `create_adapted_output`, ...) now arrive as Python exceptions instead of
  terminating the interpreter.

Fixed along the way:

- `CppModelComponentWrapper.update(required_outputs)` ignored its argument
  and always passed an empty list.
- `CppWorkflowComponentWrapper.add_model_component(component, role)` ignored
  the role.
- Both pure-Python examples had stopped instantiating at ABI 4 (no
  `value_kind` on their quantities, no `states`); nothing ran them.

### Component-loading convention upstreamed (`hydrocouplecomponentabi.h`)

The convention by which a shared library publishes a component — two
`extern "C"` entry points and an ABI stamp a host compares before calling
anything that crosses a vtable — moves into the interface package from
HydroCoupleComposer's `include/plugins/componentabi.h`, so every host and
component includes one copy instead of three. One line could not move
unchanged: the stamp's interface version was hardcoded to `2` while the
interfaces were at ABI 4, so every ABI-3/4 component stamped itself `iface=2`
and a host would have admitted an ABI-2 component beside an ABI-4 one. The
version is now a macro `static_assert`ed against `HYDROCOUPLE_ABI_VERSION`,
so bumping the interface without bumping the stamp is a compile error. The
"Toolchain" convention in `hydrocouple.h` now points at it: the interfaces are
a C++ ABI; the door a library is loaded through is the one piece defined in C.
The legacy unstamped `CreateComponentInfo` factory (used by the Python
loader) remains accepted and is reported as unstamped.

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

# Changelog

## Unreleased

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

### Fixes

- The `WorkflowStatus` renumbering above was not propagated when it landed.
  `tests/test_hydrocouple.cpp` still asserted the old values (this is the
  CI failure on Linux and macOS), and — more seriously — the Python
  bindings still declared the old nine-member enum in both
  `hydrocouple/core.py` and `_hydrocouple/_core.pxd`. Since
  `IWorkflowComponent.status` converts the C++ value through
  `WorkflowStatus(<int>…)`, a C++ `Validating` (3) would have arrived in
  Python as `Updating` (3) — silent misreporting rather than an error, with
  `Paused`/`Done`/`Finishing`/`Finished`/`Failed` raising `ValueError`.
  `python/tests/test_enum_parity.py` catches exactly this, and does fail on
  the old values; nothing in CI runs it.
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

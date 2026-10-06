# HydroCouple Interface Definitions — Critical Review

**Scope:** `hydrocouple.h`, `hydrocoupletemporal.h`, `hydrocouplespatial.h`, `hydrocouplespatiotemporal.h`, `hydrocouplespatialwkb.h` (v2.0.0, ABI 2)
**Lens:** HPC simulation, GPU acceleration, cloud/distributed execution, message passing with local virtual node representation, and computational efficiency.
**Date:** 2026-08-22

---

## 1. Summary Assessment

The interface set is a mature, OpenMI-lineage component standard: the lifecycle state machine, exchange-item pull model, adapted-output chaining, and unit/dimension metadata are well specified and documented. The v2 modernization (std::variant, std::span, `[[nodiscard]]`, removal of Qt types) is real progress.

However, the interfaces were designed around a **single-process, host-memory, scalar-access, synchronous-pull** worldview. Every one of those four assumptions breaks under HPC/GPU/cloud execution:

| Assumption baked into interfaces | Reality on HPC/GPU/cloud |
|---|---|
| Values move one `hydrocouple_variant` at a time | Fields of 10⁶–10⁹ values must move as contiguous typed buffers |
| All pointers are host memory | Data lives in device memory, pinned staging, or across nodes |
| `updateValues()` blocks until data is ready | Communication must overlap computation; pulls must be batchable and asynchronous |
| MPI ranks are the only address space | Cloud transports (TCP/gRPC/UCX), elastic workers, and failures exist |

The highest-impact gaps, in order: **(G1)** variant-boxed data path in the hot loop, **(G2)** no memory-space/device abstraction, **(G3)** MPI baked into `IModelComponent` with no transport abstraction and an anemic proxy interface, **(G4)** no partition/halo/virtual-node representation, **(G5)** pointer-chasing spatial object model with no SoA bulk access, **(G6)** no async execution or exchange semantics, **(G7)** no checkpoint/restart contract, **(G8)** no serialization/initialization contract (YAML/JSON) at the interface level.

---

## 2. Computational Efficiency (cross-cutting)

### 2.1 G1 — The variant-boxed hot path

All data movement funnels through:

```cpp
virtual void getValues(hydrocouple_variant *data,
                       std::span<const int> dimensionIndexes,
                       std::span<const int> dimensionLengths = {}) const = 0;
```

`hydrocouple_variant` holds 16 alternatives including `std::string`, `long double`, and `std::any`, so `sizeof(hydrocouple_variant)` is ≥ 48 bytes with a discriminant per element. Transferring a 1M-cell `double` field therefore:

- allocates/writes ~48 MB instead of 8 MB (6× memory traffic, cache-hostile stride);
- requires per-element construction and discriminant writes — no `memcpy`, no SIMD, no `MPI_Datatype` mapping, no `H5Dwrite` directly from the buffer, no `cudaMemcpy`;
- forces every consumer to `std::visit`/`std::get` per element on read-back.

`std::any` as an alternative additionally makes the variant non-trivially-copyable in the general case and defeats type introspection; `long double` makes the layout ABI-dependent (8/12/16 bytes across MSVC/Itanium).

**Recommendation (implemented in v2.0.0).** Backward compatibility is not a constraint, so **remove the variant convention outright** rather than layering a fast path beside it. All `getValue/getValues/setValue/setValues(hydrocouple_variant…)` overloads on `IComponentDataItem` and every descendant (id-based, temporal, spatial, spatiotemporal) are deleted and replaced by one typed bulk API; scalar access becomes a typed helper over the same path (non-normative, in `hydrocouplehelpers.h` — the interface headers themselves carry no executable code). `hydrocouple_variant` is removed entirely: former metadata accessors use native types instead (`double` bounds/missing/default values, `std::vector<std::string>` category labels with data values as category indexes, string-encoded clone arguments). The replacement:

```cpp
enum class DataKind : uint8_t { Int8, UInt8, Int16, UInt16, Int32, UInt32,
                                Int64, UInt64, Float32, Float64, String, Opaque };

struct BufferDescriptor {          // DLPack/NumPy-buffer-protocol style
  void        *data;               // host or device pointer
  DataKind     kind;
  int64_t      elementCount;
  int32_t      rank;
  const int64_t *shape;            // length == rank
  const int64_t *stridesBytes;     // nullptr => C-contiguous
  MemorySpace  space;              // §3.1
  int32_t      deviceId;
};

// On IComponentDataItem:
[[nodiscard]] virtual DataKind dataKind() const = 0;
virtual bool getValuesInto(const BufferDescriptor &dest,
                           std::span<const int64_t> start,
                           std::span<const int64_t> count) const = 0;
virtual bool setValuesFrom(const BufferDescriptor &src,
                           std::span<const int64_t> start,
                           std::span<const int64_t> count) = 0;
```

This makes exchange `memcpy`-able, SIMD/GPU/MPI/HDF5-compatible, and zero-copy when producer and consumer agree on layout, and it collapses the combinatorial per-subclass overload sets (time-index, id-index, edge/node-index variants of get/set) into `start/count` hyperslabs over well-defined dimension orderings. It also fixes **G9**: current indices are `int` (2³¹ limit) — all APIs move to `int64_t` (HDF5 `hsize_t`-compatible).

### 2.2 Virtual dispatch granularity

The efficiency rule the interfaces repeatedly violate: **virtual calls are fine per-array, fatal per-element.** Current per-element virtual accessors include `IRegularGrid2D::xNodeLocation(i,j)`, `IRegularGrid3D::zNodeLocation(i,j,k)`, `IRasterBand` reads, `IPolyhedralSurface::vertex(i)/patch(i)`, `INetwork::edge(i)`, and the entire quad-edge `IEdge` navigation (16 virtual methods returning pointers). A TIN interpolation adapter touching 10M triangles must issue ~10⁸ virtual calls and dependent pointer loads. Add bulk/SoA accessors (§5) and treat per-entity object accessors as a convenience/debug layer.

### 2.3 Signals and allocations on hot paths

`ISignal<const std::shared_ptr<IComponentStatusChangeEventArgs>&>` implies a heap-allocated event args object per status change; `IPropertyChanged` emits a `std::string` per property change. Fine at lifecycle frequency; harmful if fired per timestep or per exchange. The standard should state explicitly that value-changed and status events are *optional and must not be fired per element or per inner iteration*, and event args should be reusable/pooled. `blockSignals(bool)` is global on the signal; scoped RAII suppression would be safer.

---

## 3. GPU / Accelerator Support

### 3.1 G2 — No memory-space abstraction

Nothing in the interfaces states where a pointer lives. A GPU-resident component today must copy device→host to satisfy `getValues`, and the consumer copies host→device again — double staging on every coupling step, often the dominant cost.

**Recommendation.** Introduce a vendor-neutral space/queue vocabulary (no CUDA types in the standard, per the abstraction-layer decision):

```cpp
enum class MemorySpace : uint8_t { Host, HostPinned, Device, Unified };

struct ExecutionContext {          // opaque queue/stream handle
  void   *nativeQueue;             // cudaStream_t / hipStream_t / sycl::queue*
  int32_t deviceId;
  int32_t backendId;               // registry: 0=serial,1=openmp,2=cuda,3=hip,4=sycl
};
```

- `IComponentDataItem` gains `supportedMemorySpaces()` and `BufferDescriptor` (§2.1) carries a `MemorySpace`.
- Connection negotiation (`canConsume`/`addConsumer`) should include memory-space compatibility so a device-to-device link (same device, or peer/GPUDirect) is established once at Prepare time rather than discovered per exchange.
- `IModelComponent`'s current GPU bookkeeping (`gpuPlatform/gpuDevice/gpuMaxNumBlocksOrWorkGrps` in the SDK) is scheduler metadata, not a compute abstraction; keep it, but the data plane above is what actually enables GPU coupling.

### 3.2 Kernel-side access

Interfaces cannot be virtual on device. The standard only needs to guarantee that a component can *export* a `BufferDescriptor` whose pointer is device-resident and stable between `prepare()` and `finish()`; kernels then operate on raw spans. Document this pointer-stability contract explicitly.

---

## 4. Message Passing, Distribution, and Cloud

### 4.1 G3 — MPI baked in, and only half-in

`IModelComponent` hard-codes MPI vocabulary (`mpiProcessRank`, `mpiAllocateProcesses`, `mpiClearAllocatedProcesses`, …) yet omits the one thing an implementation needs — the communicator (`mpiCommunicator()` exists only in the SDK's `AbstractModelComponent`). Meanwhile `IProxyModelComponent` offers just three read-only accessors (`parentMpiProcessRank`, `parentProcessAddress`, `parentId`): no connect/disconnect, no data-plane contract, no status propagation, no failure semantics. `parentProcessAddress()` returning a string hints at non-MPI transports but nothing defines them.

**Recommendation.** Split *topology* from *transport* (MPI-neutral, per decision):

```cpp
class ITransport : public virtual IIdentity {      // implemented by SDK: MPI, TCP, UCX, in-proc
public:
  struct Endpoint { std::string address; int32_t rank; };   // rank = -1 for non-MPI
  [[nodiscard]] virtual Endpoint localEndpoint() const = 0;
  virtual bool send(const Endpoint &to, int32_t tag, const BufferDescriptor &payload) = 0;
  virtual bool receive(const Endpoint &from, int32_t tag, const BufferDescriptor &into) = 0;
  // async variants — see §4.3
};

class IDistributedModelComponent : public virtual IModelComponent {
public:
  [[nodiscard]] virtual ITransport *transport() const = 0;
  [[nodiscard]] virtual int32_t partitionCount() const = 0;
  [[nodiscard]] virtual int32_t partitionRank() const = 0;   // this instance's partition
};
```

Remove the `mpi*`/`gpu*` methods from `IModelComponent` (no compatibility cycle needed) and move them behind `IDistributedModelComponent`/scheduler metadata. The SDK's command-worker plan (`docs/IMPLEMENTATION_PLAN.md` §22 in HydroCoupleSDK) then implements `ITransport` over MPI inter-communicators without the *standard* naming MPI at all — which is exactly what lets the same proxy run over gRPC/UCX in a cloud cluster.

Cloud-specific gaps to cover in `IProxyModelComponent` (expanded): connection lifecycle (`connect/disconnect/isConnected`), heartbeat/liveness query, request timeout semantics, and defined behavior when the remote peer dies (proxy transitions to `Failed` with a message — currently unspecified). Elastic execution (adding/removing workers between updates) needs `IWorkflowComponent`-level rebalancing hooks; note it as future work.

### 4.2 G4 — No local virtual node representation

The standard has proxies for *whole remote components* but no representation of *remote pieces of a decomposed domain*. This is the critical missing abstraction for HPC message passing. The pattern proven in openswmm.engine is instructive on both halves:

- **Virtual junctions** (zero-DOF interface nodes): a split conduit's junction "disappears" into an interior face; its state is *reconstructed* from neighbors, never solved. Validation rules live in one place shared by every front-end.
- **1D↔2D coupling points**: what crosses the boundary is a flux plus a sensitivity (∂Q/∂h) so the receiving solver keeps implicit stability.

Generalizing, the interface standard needs a **partition/halo vocabulary**:

```cpp
class IPartitionedComponentDataItem : public virtual IComponentDataItem {
public:
  //! Global identity of locally owned entities.
  [[nodiscard]] virtual std::span<const int64_t> ownedGlobalIndexes() const = 0;
  //! Virtual (ghost/halo) entities: locally mirrored, remotely owned.
  [[nodiscard]] virtual std::span<const int64_t> virtualGlobalIndexes() const = 0;
  //! Owner partition rank of each virtual entity (parallel to virtualGlobalIndexes()).
  [[nodiscard]] virtual std::span<const int32_t> virtualOwners() const = 0;
  //! Monotonically increasing epoch, incremented on each completed synchronization.
  [[nodiscard]] virtual uint64_t synchronizationEpoch() const = 0;
};
```

Design points learned from the SWMM implementation worth encoding in the standard's prose: virtual nodes carry **no storage/DOF** (reconstruction, not duplication, avoids double-counting mass); exchanged payloads may need **flux + derivative pairs**, not just state, so coupling should not be limited to a single scalar per entity; and virtual-node **eligibility rules must be queryable** so composition tools can validate a decomposition before run time.

### 4.3 G6 — Synchronous pull only

`IOutput::updateValues(const IInput*)` is a blocking, caller-driven pull; `update()` is synchronous. There is no way to (a) post an exchange and overlap it with computation, (b) batch multiple exchange items into one message, or (c) let two components on different nodes progress concurrently. This serializes coupled executions and puts every network latency on the critical path.

**Recommendation.** Add nonblocking begin/end semantics, mirroring MPI and every halo-exchange framework:

```cpp
class IExchangeRequest {           // handle; SDK owns completion machinery
public:
  virtual ~IExchangeRequest() = default;
  [[nodiscard]] virtual bool test() = 0;
  virtual void wait() = 0;
  [[nodiscard]] virtual bool failed(std::string &message) const = 0;
};

// On IOutput:
[[nodiscard]] virtual std::unique_ptr<IExchangeRequest>
    updateValuesAsync(const IInput *querySpecifier) = 0;
// On IModelComponent (optional capability):
[[nodiscard]] virtual std::unique_ptr<IExchangeRequest>
    updateAsync(const std::vector<IOutput*> &requiredOutputs = {}) = 0;
```

Pair with a capability query (§7) so orchestrators can fall back to synchronous behavior.

### 4.4 Threading contract — unspecified

No method documents thread-safety. Minimum viable contract to add to the prose: metadata getters are safe for concurrent reads after `initialize()`; `getValues*` is safe concurrently with other `getValues*` but not with `setValues*`/`update()`; lifecycle methods are externally synchronized; signal connect/disconnect/emit is thread-safe. The single-writer IO-thread pattern (proven in openswmm.engine) depends on exactly these guarantees.

---

## 5. Spatial Interfaces

### 5.1 G5 — Object-per-entity model blocks HPC use

`IPoint`, `IVertex`, `ILineString`, `IPolygon`, quad-edge `IEdge`, `IPolyhedralSurface::patch(i)/vertex(i)` describe geometry as a web of virtual objects connected by pointers. This is expressive for editing/GIS use, but it is the *only* representation — there is no bulk, index-based view. Consequences: interpolation adapters and partitioners pointer-chase; nothing maps to UGRID persistence (which is index arrays), to GPU memory, or to `BufferDescriptor` exchange; and geometry cannot be transmitted to a remote rank without bespoke serialization (the WKB structs cover simple features but not mesh connectivity).

**Recommendation.** Add SoA/CSR bulk views alongside the object model — deliberately congruent with UGRID so persistence, exchange, and compute share one layout:

```cpp
class IMeshView {                   // obtainable from IPolyhedralSurface/ITIN/INetwork
public:
  [[nodiscard]] virtual int64_t nodeCount() const = 0;
  [[nodiscard]] virtual int64_t faceCount() const = 0;
  [[nodiscard]] virtual std::span<const double> nodeX() const = 0;   // [nodeCount]
  [[nodiscard]] virtual std::span<const double> nodeY() const = 0;
  [[nodiscard]] virtual std::span<const double> nodeZ() const = 0;   // empty if 2D
  //! CSR connectivity: faceNodeOffsets[f]..faceNodeOffsets[f+1] index faceNodes.
  [[nodiscard]] virtual std::span<const int64_t> faceNodeOffsets() const = 0;
  [[nodiscard]] virtual std::span<const int64_t> faceNodes() const = 0;
  [[nodiscard]] virtual std::span<const int64_t> edgeNodes() const = 0;  // [2*edgeCount]
};
```

`IRegularGrid2D/3D` similarly need bulk coordinate/mask spans (`xNodeLocations()` returning a span, an `activeMask()` bitset span) instead of per-node virtual calls.

### 5.2 Other spatial observations

- `IGeometry::geometryType()` enum and `hydrocouplespatialwkb.h`'s `WKBGeometryType` encode the same OGC codes twice; derive one from the other or unify to avoid drift.
- The WKB POD structs are the right idea (C-compatible, serializable); note that the unions with flexible-array-style members need explicit ownership/lifetime documentation.
- No spatial index interface (point-in-polygon / nearest-neighbor queries are every spatial adapter's inner loop) and no partitioning metadata on spatial data items (owner rank per cell — covered by §4.2 if `IPartitionedComponentDataItem` is adopted).
- `ISpatialReferenceSystem` should state its authority (EPSG code + WKT2 string) explicitly to round-trip through GeoPackage (`gpkg_spatial_ref_sys`) and CF/UGRID (`grid_mapping`).

---

## 6. Temporal Interfaces

- `IDateTime` as bare `double julianDay()` is adequate for resolution (≈ µs at contemporary dates) but carries no calendar or epoch metadata; CF/UGRID output needs `units = "days since <epoch>"` + `calendar`. Add `epoch()`/`calendar()` metadata or document the fixed convention.
- `ITimeSeriesComponentDataItem::time(int timeIndex)` returns an object pointer per step — same per-element virtual problem; add `std::span<const double> times()`.
- `timeCount()` and indexes are `int` — long simulations at small reporting steps can exceed 2³¹; use `int64_t` in new methods.
- No interface expresses *time interpolation tolerance/extrapolation policy* on temporal exchange; today it is implicit in adapted outputs. A small policy enum on temporal inputs would make composition validation meaningful.

---

## 7. Core / Lifecycle / Contract Issues

1. **Separation of concerns (G-UI):** `hasEditor()/showEditor(void*)/hasViewer()/showViewer(void*)` appear on both `IModelComponent` and `IComponentDataItem`, and `validateLicense()` on `IComponentInfo`. HPC/cloud builds must link and implement UI/licensing stubs everywhere. Move both into optional side-interfaces (`IUIProvider`, `ILicensedComponent`) discoverable by capability query; headless standard stays lean.
2. **Capability negotiation:** `HYDROCOUPLE_ABI_VERSION` exists, but a component cannot report what it supports (device buffers? async? partitioning?). Add `IModelComponent::capabilities()` returning a set of well-known capability ids; orchestrators branch on it instead of `dynamic_cast` chains.
3. **Ownership inconsistency:** `IModelComponentInfo::createComponentInstance()` returns `std::unique_ptr`, but `ICloneableModelComponent::clone()` and every factory/accessor returns raw pointers with prose-only ownership. Adopt a single rule (observers raw, transfers `unique_ptr`) and state it once, normatively.
4. **Error model:** exceptions are the only channel, which does not cross C ABI, plugin, or process boundaries (a remote worker's exception cannot propagate through a proxy). Introduce an error queue (`lastErrors()` per component, mirroring the SDK's planned ErrorQueue) and make `Failed` status + queued diagnostics the normative failure path for distributed execution; exceptions become a local convenience.
5. **State machine:** 14 statuses defined in prose only; `Failed`'s description is a duplicate of `Finishing`'s ("switches back to Created…"), and legal transitions are not tabulated. Provide a normative transition table (and add `Checkpointing`, per G7). `WaitingForData` deserves explicit iterative-coupling semantics (who retries, when is deadlock declared).
6. **G7 — Checkpoint/restart:** absent entirely, yet mandatory for preemptible cloud instances and HPC walltime limits. Add `ICheckpointableModelComponent` with `saveState(uint64_t &token)` / `restoreState(token)` / opaque state blob export, and workflow-level coordinated checkpoint in `IWorkflowComponent`.
7. **G8 — Initialization/persistence contract:** serialization should be targeted at `IArgument`. The standard already points this way — arguments are the only property accessible in `Created`, and `ArgumentInputType` enumerates `String/File/JSON/XML/URL/MEMORY_OBJECT` — but the contract is one-directional and incomplete. Make it normative and symmetric: **a component's entire persistent configuration must be expressible through its `arguments()`**, and `IArgument` gains a write-side contract to match `initialize()` — `serialize(ArgumentInputType) -> std::string` (JSON/YAML out, mirroring what the SDK's `writeData(nlohmann::json&)` already does internally) — so any driver can round-trip a component without knowing its internals. Add `YAML` to `ArgumentInputType`; fix the stale "Qt format" text on `fileFilters()`. Whole-composition load/save then needs no new serialization machinery: a composition document is just component identities + per-argument payloads + connection topology, with each component section delegated to its arguments. One efficiency requirement belongs in the contract itself: arguments can carry large field data (meshes, time series, initial conditions), and text formats must not become the bottleneck — the serialized form must support **external binary payload references** (a URI + `DataKind` + shape in the JSON/YAML, bulk bytes in a sidecar written through the `BufferDescriptor` path) so that inline text is used only below a small size threshold and round-tripping a large argument never transits a DOM.
8. **Small items:** `IQuality::categories()` returns `hydrocouple_variant_set` whose comparator treats `std::any` values as equal — two distinct `any` categories collapse; exclude `any` from category sets. `IDimension` carries only `lengthType()` — actual lengths live on the data item, so `dimensionLength(span)` has awkward "length of indexes must be one smaller" semantics; consider `std::span<const int64_t> shape()` on `IComponentDataItem`. Duplicate forward declarations (`IComponentDataItem`, `IAdaptedOutputFactory` twice) — cosmetic.

---

## 8. Prioritized Recommendations

Backward compatibility is explicitly not required, so all changes land together as **ABI 3** — one clean break instead of an additive/deprecation dance.

| # | Change | Gap | Effort |
|---|---|---|---|
| 1 | **Replace** variant get/set with `DataKind` + `BufferDescriptor` typed bulk API, `int64_t` indexing; remove `hydrocouple_variant` entirely (metadata scalars become native `double`/string types) | G1, G9 | M |
| 2 | `MemorySpace`/`ExecutionContext`; space-aware connection negotiation | G2 | M |
| 3 | `ITransport` + `IDistributedModelComponent`; **remove** `mpi*`/`gpu*` from `IModelComponent`; expand `IProxyModelComponent` (connect/heartbeat/failure semantics) | G3 | L |
| 4 | `IPartitionedComponentDataItem` (owned/virtual indexes, owners, epochs) | G4 | M |
| 5 | `IExchangeRequest` + async update/updateValues + capability query | G6 | M |
| 6 | `IMeshView` SoA/CSR bulk spans; bulk grid/time accessors | G5 | M |
| 7 | Extract UI/licensing side-interfaces; capabilities(); error queue; normative threading + state-transition tables | G-UI, contract | M |
| 8 | `ICheckpointableModelComponent`; `Checkpointing` status | G7 | S |
| 9 | `IArgument` as the normative serialization unit: symmetric `serialize()`, YAML in `ArgumentInputType`, composition load/save delegating to arguments; fix stale Qt doc text | G8 | S |

Item 1 is the keystone: with the variant data plane gone, the derived data-item interfaces shrink dramatically (most exist only to multiply get/set overloads per index kind), which lowers the cost of every other item. The companion document `SDK_MODERNIZATION_PLAN.md` in HydroCoupleSDK sequences the SDK work against these interface changes.

# HydroCouple Interface Definitions — Comprehensiveness & Robustness Review (round 2)

**Scope:** `hydrocouple.h`, `hydrocoupletemporal.h`, `hydrocouplespatial.h`, `hydrocouplespatiotemporal.h`, `hydrocoupledistributed.h`, `hydrocouplehelpers.h`, `hydrocouplespatialwkb.h` at 2.0.0-alpha.1 (ABI 2), working tree of 2026-09-29.
**Prior round:** `docs/INTERFACE_REVIEW.md` (2026-08-22). This round assumes that review's G1–G9 recommendations landed and asks two new questions: *is what is here complete and internally consistent*, and *can it be called an ontology*.
**Line references** are to the `include/` headers as of this date.

---

## 1. Bottom line

The 2.0 rewrite delivered what the August review asked for: the variant data path is gone, the typed hyperslab plane is coherent, the distributed layer is transport-neutral, the lifecycle has a table, errors have a queue, capabilities are queryable, and the new semantic enums (`ValueKind`, `TimeKind`, `VerticalCoordinateKind`, `CrossSectionKind`) are the most valuable additions in the standard's history — they encode the distinctions that make a coupling *wrong in a way that produces plausible output*, and the prose around them explains why with real incidents. The differentiable contract (`vjp`/`jvp` over `DifferentialEntry`) is well-scoped and correctly refuses to say *how* derivatives are produced.

What remains is of a different character from last time. Last time the gaps were architectural (wrong worldview). This time they are **contract-consistency defects** — places where two normative statements disagree, where a rule is stated in prose but not in the type, or where a design principle the standard proclaims is violated by its own newer additions. None requires another rewrite; most are a line or a method each. But several will bite the first independent implementer, because the two sides of a coupling will read the same header and legitimately do different things.

Ranked by consequence:

| # | Finding | Kind | §  |
|---|---|---|---|
| 1 | Lifecycle table contradicts `ICheckpointableModelComponent` docs (checkpoint from `Done`, restore after `initialize()`) and forbids `finish()` before `prepare()` | contract conflict | 3.1 |
| 2 | `BufferDescriptor` cannot describe `Opaque` (no element size) or carry `String` across ABI/wire/device; the "sole currency of exchange" has two kinds it cannot exchange | data-plane hole | 3.2 |
| 3 | Pointer ownership unspecified for ~15 geometry-producing methods; `createAdaptedOutput()` hands the caller a `unique_ptr` the provider also holds raw | lifetime hazard | 3.3 |
| 4 | Data-item entity semantics not machine-queryable: dimension roles are prose; `networkDataType()`/`meshDataType()` return the wrong enum; `MeshDataObjectType` has both `Cell` and `Face`; `IPartitionedComponentDataItem` does not say which dimension is partitioned | semantic ambiguity | 3.4 |
| 5 | No vertical datum anywhere, while three interfaces say "in the geometry's own vertical datum" | silent-error class | 4.3 |
| 6 | The standard's own "bulk not per-entity" rule is violated by `IVerticalCoordinate` (per-cell) and mixed `int`/`int64_t` widths survive across the spatial header | principle drift | 4.2 |
| 7 | Optional-interface discovery is justified by ABI stability in a release that is a deliberate ABI break, and contradicts the `capabilities()` doc that forbids `dynamic_cast` probing | rationale conflict | 3.5 |
| 8 | No semantic identifier (standard name / URI) for what is exchanged — the single largest gap between "interface" and "ontology" | ontology gap | 2, 5 |

Section 2 answers the ontology question directly. Sections 3–4 are the robustness and comprehensiveness findings. Section 5 is a prioritized list.

---

## 2. Can HydroCouple be termed an ontology for models?

**Short answer:** yes in the loose software-engineering sense, no in the knowledge-representation sense, and the honest label today is *meta-model* (or *interface standard*) *that embeds a lightweight ontology of coupled simulation*. Three targeted additions would make the stronger claim defensible.

### 2.1 What the word means in each community

Gruber's definition — "an explicit specification of a conceptualization" — is broad enough that any typed vocabulary with declared relationships qualifies. By that reading HydroCouple *is* one: it names the concepts of coupled simulation (component, exchange item, adapted output, argument, quantity, quality, unit, dimension, geometry, mesh, time series, partition, transport, workflow), fixes their relationships (is-a via inheritance; has-a via accessors; produces/consumes via `IOutput`/`IInput`), and constrains their behavior (the lifecycle table, canonical dimension orderings, the `ValueKind` regridding rules).

McGuinness's "ontology spectrum" is the usual way to place a vocabulary: catalog → glossary → thesaurus → informal is-a → formal is-a → formal instances → frames with properties → value restrictions → disjointness/inverses → general logical constraints. HydroCouple sits at **frames with properties and enumerated value restrictions**, with a few constraints expressed formally (`isValidComponentStatusTransition`) and the rest in prose. That is where UML metamodels, OpenMI 2.0, ESMF's component model and CSDMS BMI sit too. None of those is called an ontology by its authors; OpenMI calls itself an "interface standard", BMI an "interface", ESMF a "framework". The informatics community reserves *ontology* for artifacts with:

- **Formal semantics** — classes, properties and axioms in a logic (OWL/RDF/description logic) that a reasoner can check for consistency and use for inference.
- **Global identifiers** — every concept has a URI so that two systems can agree they mean the same thing without sharing code.
- **Domain concepts, not only structural ones** — a hydrology ontology says what *discharge* is and how it relates to *stage*; HydroCouple says what an *output* is and how it relates to an *input*. The domain content is left to the `id`/`caption` strings each component chooses.
- **Separation from any one implementation** — the ontology is the thing; a C++ header would be one serialization of it.

Against those criteria HydroCouple is not an ontology. It is a meta-model whose instances (components, exchange items) are C++ objects, whose relations are vtable slots, and whose domain semantics are carried by free-text identifiers.

### 2.2 Where HydroCouple already has genuine ontological content

This is worth stating because it is the part to build on, and because it distinguishes HydroCouple from OpenMI/BMI, which have none of it:

- `ValueKind` (`hydrocouple.h:2631`) — intensive/extensive/flux/density is a *semantic category that governs valid operations* (what averages, what sums, what integrates over area). That is exactly what an ontology's class distinctions are for.
- `TimeKind` (`hydrocoupletemporal.h:222`) — instantaneous vs interval-mean vs accumulated is the CF `cell_methods` concept, made a first-class type.
- `VerticalCoordinateKind` and `IVerticalCoordinate` (`hydrocouplespatial.h:1954`) — declares what a layer index *means* before values are exchanged on it; "matching shapes is not agreement" is an ontological statement.
- `CrossSectionKind` / `ICrossSection` — the flow-area/storage-area split is a conceptual distinction defended on physical grounds.
- `IUnitDimensions` — dimensional analysis (L, M, T, Θ, …) *is* the classic unit ontology, and the `conversionFactorToSI`/`offsetToSI` pair is a proper affine unit model.
- `DifferentialRole` — the (StateBefore, Input, Argument) → (StateAfter, Output) decomposition is a small, precise theory of what a "step" is.

### 2.3 What is missing for the claim to hold up

1. **No semantic identifier for the exchanged quantity.** `IValueDefinition` carries `caption`, `description`, `type_info`, `missingValue`, `defaultValue` (`hydrocouple.h:1183`), and `IQuantity` adds `unit`, `minValue`, `maxValue` — nothing says *which physical variable this is*. Two components exchanging "Q" and "discharge" cannot be matched except by a human. The SDK's UGRID writers already take a `standardName` parameter (`hdf5ugridwriter.h:73`, `netcdfugridwriter.h:71`) precisely because the interface cannot supply one. The fix is one accessor: `standardName()` returning a CSDMS Standard Name or CF `standard_name`, plus an optional `conceptURI()` so a component can point at SWEET/ENVO/QUDT/HY_Features. This single addition moves the standard from "structural meta-model" to "meta-model with a domain-ontology binding", and it also enables automatic adapter selection (unit + `ValueKind` + standard name is enough to pick or refuse an adapter).

2. **Dimension semantics are prose, not type.** Every specialization documents a "canonical dimension ordering" in a `\details` block, and `IDimension` exposes only `lengthType()`. A generic consumer (a NetCDF writer, a Python autograd driver, a partitioner) cannot ask "which axis is time?" — it must `dynamic_cast` through the specialization hierarchy and *know the prose*. Add `IDimension::role()` → `{Time, Entity, Layer, Band, Row, Column, Identifier, Component, Other}` and the orderings become checkable. (This also fixes finding 3.4.)

3. **Entity location is not aligned with the mesh vocabulary.** UGRID's `location` attribute is `node | edge | face | volume`. HydroCouple has `MeshDataObjectType {Cell, Vertex, Edge, Face}`, `NetworkDataObjectType {Node, Edge}`, and `IMeshView` speaks of nodes/edges/faces. Cell vs Face on a 2-D polyhedral surface is undefined. One enum, UGRID-named, used everywhere.

4. **No unit identifier.** `IUnit` has dimensions and an affine factor but no symbol or UDUNITS/UCUM/QUDT string, so "m3 s-1" cannot be written to a file or matched against a registry without a side table.

5. **No machine-readable export of the vocabulary.** If the concept hierarchy (the `I*` classes, the enums, the canonical orderings, the transition table) were emitted as OWL/SKOS/JSON-LD from the headers — the Python enum-parity test already parses them — the C++ would be one serialization of the ontology rather than the ontology itself, and the claim "HydroCouple defines an ontology for coupled models" would be literally true and reviewable.

### 2.4 Recommendation on terminology

In papers and the README: "HydroCouple is a component interface standard (a meta-model) for coupled simulation that embeds a lightweight ontology of exchanged quantities — their physical kind, temporal reference, vertical coordinate and units — so that compositions can be validated for *meaning*, not only for shape." That is accurate today, it is more than OpenMI/BMI can say, and it becomes "defines an ontology" once 2.3(1), (2) and (5) land. Do not call it a domain ontology of hydrology; it is not one and does not need to be — it should *bind* to those (CSDMS Standard Names, CF, QUDT, HY_Features) rather than duplicate them.

---

## 3. Robustness — contract-level defects

### 3.1 Lifecycle state machine vs. its own clients

`Helpers::isValidComponentStatusTransition()` (`hydrocouplehelpers.h:178–203`) is declared normative. Three documented behaviors are illegal under it:

| Documented | Where | Table says |
|---|---|---|
| `saveState()` "callable only when status() is Updated or Done" | `hydrocouple.h:1035` | `Done → Finishing` only; `Done → Checkpointing` rejected (`:197`) |
| `restoreState()` "callable after initialize()"; status is `Checkpointing` during it | `:1044`, `:1018` | `Initialized → {Validating, Initializing}` only (`:186`); and `Checkpointing → Updated` would skip `Validating`/`Preparing` |
| `finish()` "must be accessible after prepare()" — which is also the *only* time it is legal | `:866–877` | `Initialized`, `Valid`, `Invalid` have no edge to `Finishing` |

Consequences: the SDK's `DifferentiableWorkflow` checkpoints at `Done` (`differentiableworkflow.cpp:25`) and so already violates the table; a composition that validates `Invalid` cannot be torn down through the lifecycle (only via destructor); and a restart-from-checkpoint has no legal path. `WaitingForData` is still without iterative-coupling semantics (who retries, when is deadlock declared) — carried over from last round unchanged.

**Fix.** (a) Make `Checkpointing` return to the state it was entered from (`Updated → Checkpointing → Updated`, `Done → Checkpointing → Done`). (b) Define restore as a transition *from* `Updated` (i.e., after `prepare()`, replacing the computed state) — the semantics "behaves as if it had computed its way there" only make sense post-prepare; or, if pre-prepare restore is wanted, land in `Initialized` and require `validate()`/`prepare()` again. (c) Add `Initialized | Valid | Invalid → Finishing`. (d) Add a `WorkflowStatus` transition table; `resume()` says `Paused → Updated` but nothing else about the workflow machine is formal. (e) Consider a component-side `Prepared` state: `Preparing → Updated` overloads `Updated` to mean "prepared, nothing computed", while the workflow has a distinct `Prepared`.

### 3.2 The data plane has two kinds it cannot move

`DataKind::Opaque` — "element size must be agreed out of band" (`hydrocouple.h:114`). `BufferDescriptor` has no element-size field (`:146–155`), so `Helpers::dataKindSize()` returns 0, `contiguousByteSize()` is 0, `isContiguous()` is wrong, and `ITransport::send()` — which promises "the bytes on the wire have the same layout discipline as the bytes in memory" (`hydrocoupledistributed.h:87`) — cannot compute how many bytes to send. The one thing "out of band" defeats is the transport. **Add `int64_t itemSizeBytes`** (DLPack carries `bits`/`lanes` for the same reason); for non-opaque kinds require it to equal `dataKindSize(kind)`.

`DataKind::String` — "buffers point to arrays of `std::string`" (`:113`). `std::string` has no stable layout across compilers/stdlibs, cannot live on a device, and cannot be sent by `ITransport`. That is three of the four things the descriptor exists for. Either restrict `String` to host-only *metadata* items and say so on every `getValuesInto` doc, or adopt an Arrow-style encoding (`int64_t offsets[n+1]` + UTF-8 bytes) which is contiguous, wire-safe and NumPy-representable.

Device addressing — `MemorySpace::Device` + `deviceId` (`:127,154`) is ambiguous on a node with a CUDA and a Level-Zero device both at ordinal 0. The August review's `ExecutionContext {nativeQueue, deviceId, backendId}` (§3.1) was dropped. Without a queue/stream there is also no way to make a device `getValuesInto` asynchronous, so every device exchange serializes on the default stream. Add `int32_t backend` to the descriptor and an optional opaque `void *queue` (or a separate `ExecutionContext` parameter on `getValuesInto`/`setValuesFrom`).

### 3.3 Ownership

The August review (§7 item 3) asked for one rule — observers raw, transfers `unique_ptr` — stated once. It is stated nowhere and applied unevenly:

- Every geometry *constructor* on `IGeometry` (`boundary`, `envelope`, `buffer`, `convexHull`, `intersection`, `unionG`, `difference`, `symmetricDifference`, `locateAlong`, `locateBetween`), on `ISurface`/`IMultiSurface` (`centroid`, `pointOnSurface`, `boundaryMultiCurve`), and `IPolyhedralSurface::boundingPolygons()` returns a raw pointer to something that must be freshly allocated. No caller can know whether to delete it. (`hydrocouplespatial.h:348–515, 978–1020, 1228`)
- `IAdaptedOutputFactory::createAdaptedOutput()` returns `std::unique_ptr<IAdaptedOutput>` *and* says "the returned IAdaptedOutput will already be registered with the provider" (`hydrocouple.h:2183–2190`) — the provider now holds a raw pointer to an object the caller owns and may destroy.
- `ICloneableModelComponent::clone()` returns raw; `clones()` on the parent implies the parent tracks (owns?) them. Unstated. (`:1005–1011`)
- `IWorkflowComponentInfo::createComponentInstance()` returns raw (`:2372`); `IModelComponentInfo::createComponentInstance()` returns `unique_ptr` (`:528`).
- Two ways to wire a link: `IOutput::addConsumer()` "must and will automatically set the consumer's provider" (`:1944`) and `IInput::setProvider()` returning `bool` (`:2260`). Which is the entry point, and what happens when `setProvider` returns false after `addConsumer` succeeded?

**Fix.** One paragraph at the top of `hydrocouple.h`: accessors return non-owning observers valid for the lifetime of the object they were obtained from; anything documented as *creating* returns `std::unique_ptr`. Then apply it: geometry constructors → `unique_ptr<IGeometry>`; `createAdaptedOutput` either returns raw (provider owns) or does not register; `clone` → `unique_ptr`; workflow info → `unique_ptr`. Name `addConsumer()` the single wiring entry point and make `setProvider()` non-normative or remove it.

### 3.4 Entity and dimension semantics are not queryable, and two accessors return the wrong type

- `INetworkComponentDataItem::networkDataType()` and `IPolyhedralSurfaceComponentDataItem::meshDataType()` are documented as "the mesh entity (edge or vertex) this data item's values are attached to" but return `SpatialDataType {Scalar, MultiScalar, Vector, Tensor}` (`hydrocouplespatial.h:1730–1733, 1767–1770`). The SDK implements them as `SpatialDataType`, so the docs are wrong, and the entity question is answered by the *other* accessor (`networkDataObjectType()`/`meshDataObjectType()`).
- For `Vector`/`Tensor` items nothing declares the component dimension's ordering or basis (Cartesian x,y,z? edge-normal/tangential? row-major symmetric tensor?). A velocity exchanged between a Cartesian-grid model and an edge-normal finite-volume model is the canonical silent error and the standard cannot express the difference.
- `MeshDataObjectType` has both `Cell` and `Face` (`:57–79`). On a 2-D polyhedral surface these are the same thing; on a 3-D layered mesh they are not, and `ILayeredMeshComponentDataItem` says values are `{patch, layer}` without saying which enum value applies. `IMeshView` speaks of node/edge/face (`:1605`). Adopt UGRID's `node | edge | face | volume` in one enum and retire `Cell`/`Vertex`.
- `IPartitionedComponentDataItem` decomposes "the entity dimension" (`hydrocoupledistributed.h:271–289`). For a plain spatial item that is dimension 0; for a spatiotemporal item time is dimension 0 and the entity is dimension 1. Add `partitionedDimension()` (index into `shape()`), and state that `shape()[k] == ownedGlobalIndexes().size() + virtualGlobalIndexes().size()`.
- All of the above follow from the same root cause: `IDimension` carries no role. See §2.3(2).

### 3.5 Optional interfaces: the rationale contradicts the release

`IValueSemantics` (`hydrocouple.h:2640–2652`), `ITemporalSemantics` (`hydrocoupletemporal.h:235–238`) and `ILayering` (`hydrocouplespatial.h:2040–2044`) are separate interfaces discovered by `dynamic_cast`, and each header says this is because "adding a virtual to an existing interface changes a vtable that already-compiled components were built against". But 2.0.0-alpha.1 is a deliberate, total ABI break (`CHANGELOG.md`, "one clean break instead of an additive/deprecation dance"), and `IModelComponent::capabilities()` says "orchestrators must branch on this set rather than probing with `dynamic_cast` chains" (`hydrocouple.h:893`). Two normative statements now tell an orchestrator to do opposite things.

Also: `Capability` is a closed `enum class : uint32_t` (`:162`). Third parties cannot declare a capability without editing the standard. Either reserve a vendor range (`0x8000_0000+`) or key capabilities by string.

**Fix.** Pick one. Given the alpha status, put `valueKind()` on `IValueDefinition` and `timeKind()`/`intervalLength()` on `ITimeSeriesComponentDataItem` (with `Unknown` as the default answer) — they are not optional in any composition that wants to be validated. Keep `ILayering` as a mixin (it genuinely is one) but add `Capability::LayeredData`. Rewrite the three rationales.

### 3.6 Error channels

Three coexist: exceptions (`initialize()` "an exception will be thrown", `:773`; `addConsumer` "an exception will be thrown", `:1941`), return-`bool` + `message` (data plane, checkpoint, transport), and the `errors()` queue declared normative (`:900–909`), plus `validate()` returning a `vector<string>` that is *not* said to be mirrored into the queue. State once: lifecycle methods set `Failed`, queue a `Fatal` entry, and *may additionally* throw; `validate()` messages are queued at `Warning`/`Error`; `bool`+`message` methods must also queue at `Error`. Otherwise a proxy forwarding a remote `validate()` loses the messages.

### 3.7 Smaller robustness items

- `IMultiInput::canConsume(IOutput*, std::string&, const IIdentity* = nullptr)` hides `IInput::canConsume(IOutput*, std::string&)` (`hydrocouple.h:2268, 2307`) — `-Woverloaded-virtual`; every implementer must implement both, and the default argument makes call resolution non-obvious. Rename (`canConsumeAs`) or add `using IInput::canConsume;` in the interface.
- `IGeometry::relate(const IGeometry&)` (`hydrocouplespatial.h:451`) is documented with an `intersectionPatternMatrix` it does not take. OGC `Relate(g, pattern)`.
- `IExchangeRequest` destructor: "cancels if supported, otherwise blocks" (`hydrocoupledistributed.h:57–59`) — two behaviors under one call; a caller cannot write correct code. Always-wait plus an explicit `cancel()` is unambiguous.
- `IProxyModelComponent` promises `status()` "kept current asynchronously from the peer's status notifications" (`:203`) while the core threading contract says lifecycle methods require external synchronization. State that `status()` is atomic/thread-safe on every component.
- `IWorkflowComponent::requestStop()` "safe to call from a signal handler thread" (`hydrocouple.h:2504`) — async-signal-safety is a much stronger property than thread-safety and almost certainly not what is meant.
- STL containers (`std::vector`, `std::set`, `std::string`, `std::unordered_map`, `std::shared_ptr`) cross the plugin boundary by value throughout. This binds every component and host to one compiler/stdlib/CRT and precludes the "C-ABI shim" that `DifferentialEntry` mentions (`:1072`). Acceptable, but say it normatively ("same-toolchain rule") — and note the visibility pragma is GCC/Clang only (MSVC needs `__declspec` or relies on name-based RTTI comparison).
- `IValueDefinition::type()` (`std::type_info`, `:1197`) and `IArgument::validComponentDataItemTypes()` (`:1815`) duplicate `dataKind()` with a less portable mechanism.
- `IQuality`: data values are indexes into `categories()` (`:1275`) — say which `DataKind` a quality item must report.
- `IDateTime : IPropertyChanged` makes every time value a signal with `connect/disconnect/blockSignals`; `ITimeSpan : IDateTime` makes a span *be* an instant (its start) with no explicit `end()`; `serialDate()` names no epoch (Excel 1900? MATLAB datenum? Unix days?). (`hydrocoupletemporal.h:52–93`)
- `TimeKind`/`ITemporalSemantics` are declared in `HydroCouple`, not `HydroCouple::Temporal` (`hydrocoupletemporal.h:207–257`; the indentation suggests an accident).
- `IExchangeItemChangeEventArgs` (`hydrocouple.h:1874`) is declared but no interface emits it; `IUnit::AreaUnits` is referenced by nothing in the standard. Both violate "no speculative abstraction".
- Stale prose: `inputs()`/`outputs()` "accessible after initialize() and until validate()" (`:725–729, 741–745`) — the pull model reads them during `update()`; `Preparing` "for the first `getValue()` call" (`:613`); `IValueDefinition` "returned by `getValue()`" (`:1177`); `RasterDataType::ARGB32` "same as `QImage::Format_ARGB32`" (`hydrocouplespatial.h:1300–1303`).

---

## 4. Comprehensiveness — what a coupled water-resources model still cannot say

### 4.1 Time

- `ITimeModelComponent` exposes `currentDateTime()` and `simulationPeriod()` only (`hydrocoupletemporal.h:99–118`). There is no `timeStep()`, no `nextTime()`/"earliest time I can provide", no "latest time I can accept". A scheduler building a time-stepped composition (the SDK's `TimeSteppedWorkflow`) has to guess or configure out of band what the standard could state.
- An `IInput` cannot say *which time* it needs except by *being* an `ITimeSeriesComponentDataItem` whose `times()` are read by the provider inside `updateValues(const IInput*)`. That is implicit and undocumented on `updateValues` (`hydrocouple.h:2000–2013`). Say it.
- No interpolation/extrapolation policy on temporal inputs (August §6, still open): may the provider extrapolate? hold last value? refuse? A small enum on `ITimeSeriesComponentDataItem` (or on `ITemporalSemantics`) makes `validate()` able to reject a composition that would silently extrapolate.
- Layered items have no spatiotemporal counterpart (no `ITimeLayeredMesh…`/`ITimeLayeredNetwork…`), and `ILayering` says the layer dimension is "last" — with time at 0 and entity at 1 that needs stating. `IVerticalCoordinate::isTimeVarying()` exists but the elevations carry no epoch/time stamp, so a consumer cannot tell whether the profile it holds is current (contrast the good `synchronizationEpoch()` design in the distributed header).

### 4.2 Space and mesh

- `IMeshView` (`hydrocouplespatial.h:1605–1660`) has node coordinates, face→node CSR and edge→node pairs. Missing, and needed by every conservative remap that `ValueKind::Extensive`/`Flux` mandates: face→edge, edge→face (with boundary marker), face centroids, face areas, edge lengths and normals. UGRID defines `face_edge_connectivity`, `edge_face_connectivity`, `face_coordinates`; consumers will otherwise recompute these inconsistently on each side of a coupling. Add the two connectivities as required and the metric quantities as optional bulk spans (empty if not provided).
- `IVerticalCoordinate::interfaceElevations(cellIndex, double*)` is per-cell (`:2033`). For a time-varying sigma mesh with 10⁶ columns that is 10⁶ virtual calls per exchange — the exact pattern August §2.2 said is "fatal per-element". Add a bulk `[cell][interface]` accessor (span or `BufferDescriptor`).
- Index widths (August G9 asked for `int64_t` everywhere): `int` survives in `IGeometryCollection::geometryCount/geometry`, `ILineString::pointCount/point`, `IPolygon::interiorRingCount/interiorRing`, `IRaster`/`IRasterBand` sizes and offsets, `IRegularGrid2D/3D` node counts and per-node accessors, `IVerticalCoordinate::layerCount`; `unsigned int` in `IGeometry::index`, `IVertex::index`, `IEdge::index`. A 50 000 × 50 000 DEM (2.5·10⁹ cells) overflows the raster API today.
- `IRasterBand::read/write(…, void*)` (`:1384, 1394`) is a second, untyped data path beside `BufferDescriptor`, and `RasterDataType` (`:1274`) is a second element-type vocabulary beside `DataKind` (with complex and ARGB kinds that `DataKind` lacks). A `CFloat64` band cannot be exchanged through an `IRasterComponentDataItem` hyperslab. Either route bands through the descriptor or document the mapping and what happens for the unmappable kinds.
- `SpatialDataType::Vector/Tensor` have no basis or component ordering (see §3.4).
- No spatial index or point-location query (`containingCell(x, y)`, nearest node). Every spatial adapter's inner loop; still absent from the standard, so each adapter builds its own. Acceptable if deliberate — say so.
- `ICrossSection` hardcodes metres (`:2149–2177`) while `ISpatialReferenceSystem::distanceUnits()` may say feet; and its per-stage virtual accessors sit in a 1-D solver's Newton loop. A tabulated bulk form (`table(stages[], areas[], widths[], perimeters[])`) would let a partner build its own lookup once.

### 4.3 Reference systems

`ISpatialReferenceSystem` (`hydrocouplespatial.h:130–160`) is horizontal only: authority, SRID, WKT, distance units. `IVerticalCoordinate` ("positive up, in the geometry's own vertical datum", `:1985`), `ICrossSection` ("in the geometry's own datum", `:2122`) and `IMeshView::nodeZ()` all assume a vertical datum that nothing declares. NAVD88 and NGVD29 differ by ~0.3 m across much of the US; coupling a river model on one to a groundwater model on the other produces plausible output and no error — the failure class this standard now explicitly hunts. Add `verticalAuthName()/verticalSRID()/verticalSRText()` (a compound CRS) or at minimum a vertical-datum string and vertical units, and let `validate()` compare them.

### 4.4 Units and quantities

- `IUnit` has no symbol / UDUNITS / UCUM / QUDT identifier (`hydrocouple.h:1380–1546`). Files and registries need one; the SDK writers again need a side table.
- `FundamentalUnitDimension` lacks plane angle (radians are dimensionally pure but every geodetic/`Degrees` quantity needs it) and treats `Unitless` as a dimension (it is the all-zero vector, not a basis element). `Currency` as a base dimension is defensible for water-economics coupling — keep it, but say why.
- `IUnitDimensions::power()` is non-`const` (`:1374`).
- `DistanceUnits::Degrees` mixes angular into linear units (`:1432`).

### 4.5 Component and composition concepts still absent

These are the conceptual pieces a *modeling* ontology would be expected to name and that the standard does not:

- **State variables** as a first-class list on `IModelComponent`. `differentiableStates()` exists only on `IDifferentiableModelComponent`; checkpointing, data assimilation, ensemble generation and restart all need "what is your state" without differentiation. Promote `states()` to `IModelComponent` (may be empty) and let the differentiable subset refer to it.
- **Parameter vs. forcing vs. initial condition** among arguments. `IArgument` is one bag; a calibration driver, a DA driver and a scenario generator each need to partition it and each will invent its own tag. A `role()` on `IArgument` (`Parameter`, `InitialCondition`, `Forcing`, `Configuration`) is the ontological distinction.
- **Boundary condition** — the concept every hydrological coupling is actually about (a flux or state imposed at a domain boundary) has no name in the standard; it is an `IInput` like any other. A `BoundaryRole` on inputs (`Dirichlet`, `Neumann`, `Robin`/flux-sensitivity) is what lets a composer validate that a 1-D/2-D coupling exchanges *flux + ∂Q/∂h* rather than a bare state — which the distributed header prose recommends (`hydrocoupledistributed.h:279–282`) but nothing can check.
- **Conservation / budget reporting.** Nothing lets a component report mass/energy balance closure, so a workflow cannot verify that a coupling conserves. One optional interface (`IConservationReport`: per-quantity inflow/outflow/storage-change/residual since last call) is small and makes `ValueKind::Extensive` auditable.
- **Ensemble / realization dimension** and **uncertainty** metadata — absent; the `IDimension::role()` enum from §2.3(2) is the natural home for `Realization`.
- **Provenance** on `IComponentInfo` beyond developer/version/citations: no content hash of the library, no build identifier — needed for reproducibility claims in cloud execution.
- **Collectives on `ITransport`.** Only point-to-point exists (`hydrocoupledistributed.h:94–160`). Iterative-coupling convergence checks and global budgets need `barrier()` and `allReduce(min/max/sum)`; without them the SDK must call MPI directly and transport neutrality is lost exactly where it matters. Either add two methods or state that collectives are out of scope and the SDK owns them.

### 4.6 What is covered well (for balance)

Lifecycle vocabulary; pull-based exchange with adapted-output chaining; multi-provider inputs; the typed hyperslab plane and its helpers; capabilities and error queue; checkpoint contract shape; the differentiable contract; transport/proxy/partition trio with epoch-stamped halos; OGC SFA geometry incl. WKB PODs; UGRID-congruent `IMeshView`; regular-grid bulk spans; the four semantic enums; `IArgument` as the serialization unit with external binary payload references; threading contract on data items; RTTI-visibility handling for cross-image `dynamic_cast`; Python parity tests parsing the headers.

---

## 5. Prioritized recommendations

Effort: S = a few lines, M = one interface or a coordinated doc/enum change, L = touches several headers and the SDK.

| # | Change | Fixes | Effort |
|---|---|---|---|
| 1 | Reconcile the lifecycle table with checkpoint/finish docs; add workflow table; decide on component `Prepared` | 3.1 | S |
| 2 | `BufferDescriptor.itemSizeBytes` (+ `backend`, optional `queue`); redefine or restrict `DataKind::String` | 3.2 | M |
| 3 | One ownership rule, stated once; `unique_ptr` from every creator; single link-wiring entry point | 3.3 | M |
| 4 | `IDimension::role()`; one UGRID-named entity-location enum; fix `networkDataType`/`meshDataType` docs; `partitionedDimension()`; vector/tensor basis | 3.4, 2.3(2–3) | M |
| 5 | `standardName()` + `conceptURI()` on `IValueDefinition`; `symbol()`/`ucum()` on `IUnit` | 2.3(1,4), 4.4 | S |
| 6 | Vertical CRS/datum on `ISpatialReferenceSystem`; `validate()` compares | 4.3 | S |
| 7 | Move `valueKind`/`timeKind` onto their base interfaces (or add capabilities) and rewrite the three ABI rationales; open the `Capability` range | 3.5 | S |
| 8 | `IMeshView`: face↔edge, edge→face, optional metrics; bulk `IVerticalCoordinate` accessor; `int64_t` sweep of the spatial header | 4.2 | M |
| 9 | Time: `timeStep()`/`nextTime()` on `ITimeModelComponent`; interpolation policy; document the query-specifier contract on `updateValues`; time-layered items | 4.1 | M |
| 10 | Single error-channel statement; `validate()` messages queued | 3.6 | S |
| 11 | `states()` on `IModelComponent`; `IArgument::role()`; boundary role on inputs; optional `IConservationReport`; `barrier`/`allReduce` on `ITransport` (or declare out of scope) | 4.5 | L |
| 12 | Cosmetic sweep: `relate()` pattern arg, `canConsume` hiding, namespace of `TimeKind`, unused `IExchangeItemChangeEventArgs`/`AreaUnits`, stale OpenMI/Qt prose, `power()` const, `IExchangeRequest` destructor semantics | 3.7 | S |
| 13 | Generate an OWL/SKOS/JSON-LD vocabulary from the headers in CI (alongside the Python enum-parity parser) | 2.3(5) | M |

Items 1–4 are correctness; an implementer today can read the headers and build something that is either illegal or unsafe. Items 5–7 are cheap and are what separate "interface standard" from "ontology-bearing standard". The rest is completeness.

---

## 6. Status (2026-09-29, same day)

Robustness (§3) and comprehensiveness (§4) items were implemented in HydroCouple (ABI 4; see CHANGELOG "Contract-consistency round") with the SDK updated in lockstep (`plans/sdk/ABI4_HANDOFF_2026-09-29.md`). Ontology items (§2, §5 #5 and #13) are **deliberately held back** at the author's request.

| # | Recommendation | Status |
|---|---|---|
| 1 | Lifecycle table reconciled; workflow table; no component `Prepared` (documented instead) | done |
| 2 | `itemSizeBytes`, `backend`, `queue`; `String` restricted to host metadata | done |
| 3 | Ownership rule stated once; `unique_ptr` creators; `addConsumer()` single entry point; adapter self-deregistration | done |
| 4 | `IDimension::role()`; `MeshLocation`; `entityDimension()`; `vectorBasis()`; `partitionedDimension()` | done |
| 5 | `standardName()`/`conceptURI()`/unit symbols | **held (ontology)** |
| 6 | Vertical CRS on `ISpatialReferenceSystem`; cross-section/vertical coordinate refer to it | done; the SDK's connection validator now compares horizontal and vertical reference systems (codes 1107/1108, 2026-10-01) |
| 7 | `valueKind()` on `IValueDefinition`, `timeKind()` on time-series items; rationales rewritten; vendor capability range | done |
| 8 | `IMeshView` face↔edge, edge→face, metrics; bulk `IVerticalCoordinate`; `int64_t` sweep | done |
| 9 | `nextDateTimeJulianDay()`; interpolation/extrapolation policy; query-specifier contract; time-layered items | done |
| 10 | Single error-channel statement; `validate()` messages queued | done |
| 11 | `states()` and `IArgument::role()` done; **collectives, boundary role, conservation report deferred** (no consumer yet — the standard's own rule) | partial, by design |
| 12 | Cosmetic sweep (`relate()`, `canConsume` hiding, namespace, unused types, stale prose, `power()` const, request destructor) | done; also found and fixed `IProxyModelComponent::connect()` hiding the signal `connect()` |
| 13 | OWL/SKOS export | **held (ontology)** |

Found during implementation, not in the review: the two-`ISignal<>` name hiding on components/workflows (using-declarations added); the Python enum parser cannot tell two nested enums named `Role` apart (named `DimensionRole`/`ArgumentRole`); the Python mirror never carried `IVerticalCoordinate`, `ILayering`, `ICrossSection` (added 2026-10-01; see below).

### Update (2026-10-01)

Shipped to the working trees, not yet verified on macOS or committed
(`plans/sdk/ENFORCEMENT_HANDOFF_2026-10-01.md`,
`plans/hydrocouple/BINDINGS_CONFORMANCE_HANDOFF_2026-10-01.md`):

- **The SDK reads the ABI-4 declarations.** `setStatus()` enforces the
  transition tables (code 1001); a connection validator, run by
  `AbstractWorkflowComponent::validate()` over every link, compares data
  kind, value definition, `ValueKind`, unit, `TimeKind`, interpolation and
  extrapolation policy, horizontal and vertical reference systems, mesh
  location, vector basis, layering and dimension roles (codes 1101–1112;
  Error when both ends declare and contradict, Warning when one is silent).
- **Lifecycle:** `Failed` is reachable from every status but `Finished`
  (the enforcement surfaced the contradiction with `IProxyModelComponent`).
- **Python mirror:** the vertical-structure interfaces and their bindings;
  transition tables brought from ABI 2 to ABI 4; every ABC checked member
  for member against its C++ header; every Cython wrapper held to the ABC it
  is registered as (the `IWorkflowComponent` ABC had missed its six ABI-4
  members).

Still open: #5 and #13 (held, ontology); the rest of #11 (collectives,
boundary role, conservation report — waiting for a consumer); a
Python-implemented spatial or layered item is not yet visible to C++ as
such (the bridge carries the data plane only).

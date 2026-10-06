# Composition IO and Stored Results — Plan of Attack

**Status:** IMPLEMENTED (all five slices) · **Target:** HydroCoupleSDK v2.1.0 (additive; no v2.0 ABI break)
**Author:** Caleb Buahin

> **Implementation note (complete).** All five slices landed:
> `16ef101` Composition Specification v1 + JSON Schemas,
> `973dc94` writer catalogs + run manifests,
> `4514c0d` result readers + ResultsModelComponent,
> `8e579f9` execution modes + TimeSliceOutput,
> `c6f07fb` analysis example + docs + install fixes,
> plus `a57c5f8` in the interface repo (Finished doc clarification).
> Suite: 203 green with IO features and examples, 194 lean.
>
> Two things the plan did not foresee, both now handled: an item recorded
> by several writers is one logical variable (exposed once, from the most
> capable readable format), and a recorded item is rank-2 `{time, …}`
> while a live input is rank-1 — bridged by the new `TimeSliceOutput`
> rather than by guessing which level is "now".

---

## 1. Problem statement

Two gaps remain after the v2.0.0-alpha.1 modernization:

1. **Composition IO lives outside the SDK.** HydroCoupleComposer owns the
   composition format (XML) and its persistence logic. Every other host —
   a CLI runner, a Python driver, a test harness, a cloud worker — has to
   reimplement it or depend on a GUI. The format is also opaque: hand-editing
   and diffing XML compositions is unpleasant, and nothing validates a
   document before it is applied.

2. **Completed runs are a dead end.** Once a composition finishes, its
   results exist only as files. There is no way to reopen a run, enumerate
   what was produced, or feed those results to analysis and visualization
   tools — least of all to an *output component that never participated in
   the coupling*. Today a visualization UI would need each model's binary
   just to read that model's numbers.

This plan closes both, in the SDK, with JSON/YAML documents that are
schema-validatable, and with a stored-run representation that makes
previously executed compositions first-class citizens.

---

## 2. Design decisions (and what they reject)

### D1 — The composition document is an SDK-owned, versioned specification

The `ModelInitializer` schema (components / arguments / connections) becomes
**Composition Specification v1**: one JSON object model, YAML accepted as a
pure syntax alternative (it already converts to JSON at the boundary, so a
single schema governs both). XML is not carried forward.

*Rejected:* keeping the format in the Composer and exposing a converter.
That leaves two sources of truth and keeps headless hosts second-class.

### D2 — Validation is two-layered, and the runtime layer is authoritative

- **Formal layer:** JSON Schema (2020-12) documents shipped and installed
  under `share/hydrocouplesdk/schema/`, referenced from documents via
  `$schema`. Editors autocomplete, CI lints, external tools validate — with
  no C++ involved.
- **Runtime layer:** the existing validate-before-apply pass, extended to the
  new fields, remains the authority. It knows things a schema cannot: whether
  component id `x` resolves, whether argument `coeffs` exists on it, whether
  an output's `DataKind` and shape can feed the target input.

*Rejected:* schema-only validation (cannot see live components) and
runtime-only validation (no tooling story, no editor support).

### D3 — A finished run is described by a **run manifest**, not inferred from disk

At `finish()`, an opt-in recorder writes `run.json`: what was run, by which
versions, with what final statuses, and — critically — a **results catalog**
mapping each recorded data item to the artifact, variable, kind, shape,
units, mesh, location, and time span that hold its values.

*Rejected (explicitly):* letting components sniff their own output directory
and decide they are "already done". Silent reuse of stale results is a
reproducibility hazard: the same document would run or not-run depending on
filesystem state. The manifest makes reuse **declared, versioned, and
hashable**.

### D4 — Reopening a run needs no model binary: `ResultsModelComponent`

A stored run is reopened as an SDK-provided component constructed from a
manifest entry. It adopts the original component's identity, exposes the
recorded items through `results()` **and** `outputs()`, serves values by
reading hyperslabs directly from the artifacts, and — per the user's
inclination — reaches `Finished` during `initialize()`, having passed
through the documented status sequence.

This is where `IModelComponent::results()` finally gets a first-class
consumer: outputs that were recorded for analysis but never coupled.

*Rejected:* a bespoke "results API" separate from the component model. Making
stored results *be* a component means every existing consumer — workflows,
adapted outputs, the data plane, a UI's component browser — works unchanged.

### D5 — Execution intent is explicit in the document

Each component block may carry
`execution: { mode: "run" | "open" | "resume" }`:

| mode | meaning |
|---|---|
| `run` (default) | compute normally; never silently reuse results |
| `open` | load recorded results and finish immediately (D4 applies; a live component may implement this itself, otherwise the SDK substitutes a `ResultsModelComponent`) |
| `resume` | restart from a checkpoint (reserved; ties to `ICheckpointableModelComponent`) |

A manifest reopened for analysis naturally yields an all-`open` document.

### D6 — Composition documents are executable content; loading stays opt-in

Documents may name shared libraries (`info.library`). Resolution therefore
stays **caller-supplied by default** (the existing `ComponentResolver`). Any
SDK-provided loader is opt-in, takes an explicit allow-list of search
directories, and never loads during *validation* — only during *apply*.

---

## 3. Deliverables

### 3.1 Composition Specification v1

```jsonc
{
  "$schema": "https://hydrocouple.org/schema/composition-1.0.json",
  "schema_version": "1.0",

  "metadata": {
    "id": "boyne-catchment-study",
    "caption": "Boyne catchment coupling",
    "description": "…",
    "author": "…",
    "created": "2026-08-23T12:00:00Z",
    "sdk_version": "2.0.0-alpha.1",
    "interface_version": "2.0.0-alpha.1"
  },

  "components": [
    {
      "id": "catchment",
      "caption": "Catchment",
      "info": {                       // optional: lets a UI round-trip
        "library": "libcatchment.so", // instantiation without a resolver
        "component_info_id": "CatchmentComponentInfo"
      },
      "execution": { "mode": "run" },
      "arguments": {
        "coefficients": { "values": [0.35, 0.20, 0.55] }
      }
    }
  ],

  "connections": [
    {
      "from": {
        "component": "catchment",
        "output": "runoff",
        "adapted_outputs": [          // ordered adaptation chain
          { "factory": "TemporalInterpolationFactory",
            "id": "linear-interp",
            "arguments": { "method": { "value": "linear" } } }
        ]
      },
      "to": {
        "component": "channel",
        "input": "lateral_inflow",
        "role": null                  // IMultiInput provider role id
      }
    }
  ],

  "workflow": {
    "strategy": "time_stepped",       // or "pull_driven"
    "trigger": { "component": "channel", "input": "lateral_inflow" },
    "iterations_per_group": 1
  },

  "writers": [
    { "type": "hdf5_ugrid", "path": "out/results.h5", "mesh": "reaches" },
    { "type": "geopackage", "path": "out/results.gpkg",
      "simulation": "boyne-run-1",
      "fields": [ { "item": "flow", "units": "m3/s", "description": "…" } ] }
  ],

  "run": { "manifest": "out/run.json" }   // where the recorder writes
}
```

Backward compatibility: every new key is optional. Existing two-key documents
(`components`, `connections`) remain valid and are treated as
`schema_version: "1.0"` with defaults.

### 3.2 Run manifest v1

```jsonc
{
  "$schema": "https://hydrocouple.org/schema/run-1.0.json",
  "schema_version": "1.0",

  "run": {
    "id": "b0f2…",  "started": "…Z", "finished": "…Z",
    "wall_seconds": 12.4, "status": "completed",   // completed | failed | stopped
    "host": "…"
  },
  "versions": { "sdk": "2.0.0-alpha.1", "interface": "2.0.0-alpha.1" },

  "composition": {                   // embedded copy or reference + hash
    "path": "composition.json",
    "sha256": "…",
    "embedded": { /* optional full document */ }
  },

  "components": [
    { "id": "channel", "final_status": "Finished", "errors": [] }
  ],

  "results": [
    {
      "component": "channel",
      "item": "flow",
      "artifact": "results.h5",       // relative to the manifest
      "format": "hdf5_ugrid",
      "variable": "flow",
      "kind": "Float64",
      "shape": [24, 3],
      "dimensions": ["time", "face"],
      "units": "m3/s",
      "description": "Routed channel flow",
      "mesh": "reaches",
      "location": "face",
      "time": { "count": 24, "start": 1.0, "end": 24.0, "units": "julian_day" }
    }
  ]
}
```

### 3.3 Writer catalog contract

`FieldSlice` carries `itemId`, `kind`, `shape` — but units, mesh, and
location live writer-side (`GeoPackageWriter::addField`, the UGRID writers'
`MeshDefinition`). So the writers report what they wrote:

```cpp
// io/outputwriter.h — additive, non-breaking
struct ResultEntry { /* the fields of §3.2 "results" */ };

/*! Catalog of what this writer produced; valid after finalize().
 *  Default: empty (a writer that does not describe its artifacts). */
[[nodiscard]] virtual std::vector<ResultEntry> catalog() const { return {}; }
```

Each shipped writer implements it. The default keeps third-party writers
compiling untouched.

### 3.4 `RunRecorder`

Collects run metadata, drains writer catalogs at finalize, and writes the
manifest. Wired into the workflow's `finish()` when configured, or driven
manually by a host that owns its own loop.

### 3.5 Result readers

```cpp
class IResultReader {
  virtual bool open(const std::filesystem::path &, std::string &message) = 0;
  virtual bool read(const ResultEntry &, std::span<const int64_t> start,
                    std::span<const int64_t> count,
                    const BufferDescriptor &into, std::string &message) = 0;
};
```

| reader | gate | notes |
|---|---|---|
| `CSVResultReader` | always | long format `time,item,index,value`; index built on open |
| `HDF5ResultReader` | `USE_HDF5` | native hyperslab reads |
| `NetCDFResultReader` | `USE_NETCDF` | native hyperslab reads |
| `GeoPackageResultReader` | `USE_GEOPACKAGE` | `result_timeseries` → (time, object_index) reshape |

A small registry maps `format` strings to readers, so the manifest is the
only coupling between a run and the code that reads it.

### 3.6 `ResultsModelComponent`

- Constructed from a manifest + component id (or a filtered entry list).
- `initialize()`: opens readers, builds one lazily-backed item per catalog
  entry, then `Initializing → Initialized → Finishing → Finished`, with a
  status message naming the run id.
- `results()` returns every recorded item; `outputs()` returns them as
  `IOutput`s so a stored run can drive a **new** composition as a boundary
  condition (the workflows already skip `Done`/`Finished` components while
  still calling `updateValues()` on their outputs — this works today, with a
  time-interpolating adapted output for step alignment).
- Items implement the v2 data plane by delegating `getValuesInto()` to the
  reader, plus `ITimeSeriesComponentDataItem` when the entry has a time
  dimension. Values are read on demand; an optional per-item cache keeps
  repeat scrubbing in a UI cheap.

### 3.7 Interface documentation clarification

`ComponentStatus::Finished` currently reads as "released resources and cannot
be restarted". Add one clarifying sentence: components release *computational*
resources; data items of a results-backed component remain readable in
`Finished`. **Doc-only — no signature change, no ABI impact.**

---

## 4. Execution plan

Each slice states its verification criterion, per the repository's
goal-driven convention.

### Slice 1 — Composition Specification v1 + schemas

1. Extend the document model: `schema_version`, `metadata`, `info`,
   `execution`, `adapted_outputs`, multi-input `role`, `workflow`, `writers`,
   `run` → verify: existing two-key documents still initialize (no
   regression in `test_io.cpp`), new keys round-trip through `serialize()`.
2. Author `composition-1.0.json` and `run-1.0.json` JSON Schemas; install to
   `share/hydrocouplesdk/schema/` → verify: the example compositions and a
   generated manifest validate against them (Python `jsonschema` in CI).
3. Extend runtime validation with actionable messages for every new field →
   verify: one negative test per failure mode (unknown role, unknown
   adapted-output factory, bad workflow strategy, malformed writer block).

### Slice 2 — Writer catalogs + `RunRecorder`

1. Add `ResultEntry` and `IOutputWriter::catalog()`; implement in CSV,
   HDF5/UGRID, NetCDF/UGRID, GeoPackage writers → verify: each writer's
   catalog matches what an independent reader finds in the artifact
   (h5py / sqlite3 / NetCDF C API, as the existing writer tests already do).
2. `RunRecorder`: metadata, component statuses, catalog aggregation, manifest
   write; optional composition embedding + sha256 → verify: manifest
   validates against the schema and lists every field written.
3. Wire into `AbstractWorkflowComponent::finish()` behind configuration →
   verify: the serial example emits `run.json` with 2 components and 2 items.

### Slice 3 — Readers + `ResultsModelComponent`

1. Reader interface + registry + CSV reader → verify: reads back the CSV the
   IO thread wrote, hyperslab-exact.
2. HDF5, NetCDF, GeoPackage readers behind their existing gates → verify:
   per-format round trip write → read → compare, bit-for-bit for float64.
3. `ResultsModelComponent` + lazily-backed items → verify: **the headline
   test** — run the serial example, reopen from `run.json`, and assert values
   read through the data plane are byte-identical to the live run's, with no
   model component linked into the test binary for the reopened side.
4. Status sequence and diagnostics → verify: reaches `Finished` through the
   documented sequence; a missing artifact yields `Failed` plus a `Fatal`
   error entry naming the file.

### Slice 4 — Execution modes and composition-level reuse

1. `execution.mode` handling in `ModelInitializer`: `open` substitutes a
   `ResultsModelComponent` (or defers to a component that implements loading
   itself); `run` forbids implicit reuse; `resume` reserved and diagnosed →
   verify: a document with `mode: open` initializes a composition whose
   upstream component is Finished before any update.
2. Stored run as a boundary condition → verify: a two-component composition
   where upstream is a reopened run and downstream computes fresh produces
   the same downstream values as the fully live run.
3. Manifest → composition round trip (`compositionFor(manifest)`) → verify:
   the emitted document validates and reproduces the analysis view.

### Slice 5 — Examples, docs, release hygiene

1. Third example, `analysis_reopen`: runs the serial composition, then
   reopens it and prints a summary from `results()` alone → verify: builds
   standalone against the installed SDK and runs in CI.
2. Doxygen for all new headers; the interface doc clarification (§3.7);
   Readme section on composition documents and stored runs → verify: docs
   build clean.
3. `CHANGELOG.md` entry; schema files in the install and CPack payload →
   verify: install + CPack contain `share/hydrocouplesdk/schema/*.json`.

### Slice 6 — Recorded items carry the geometry their entries name ✅ DONE 2026-08-26

**Gap found from the consuming side (HydroCoupleComposer, 2026-08-26).** A
reopened run's items answer for their values and their time axis and nothing
else: `RecordedItem` is an `AbstractOutput` plus
`ITimeSeriesComponentDataItem`, and `IResultReader` reads values and times
only. Every entry in the catalog *names* what its values are attached to —
`mesh: "reaches"`, `location: "node"` — and the artifacts themselves carry the
geometry, but nothing reads it back. So a tool that opens a run can plot it
and cannot map it, and `IPolyhedralSurfaceComponentDataItem` is false for
every recorded item there is.

This is §3.6 finishing what §3.3's catalog contract already records. The
mesh attachment was written into the manifest so a reader would not have to
infer it; until it is read, that field is a comment.

**Not the consumer's job.** The alternative is for each UI to open the
artifact a second time and pair the geometry with the values itself. That
duplicates the reader, puts the manifest's mesh field in two hands, and
leaves the geometry invisible to everything else that reopens a run — a
plot that wants to label a series by location, a component that drives a new
composition from a recorded boundary. Geometry belongs to the item.

**What each format can answer.** "All four" is three that carry geometry and
one that is honest about carrying none:

| format | artifact holds | geometry served |
|---|---|---|
| `csv` | values only | none — and the entry names no mesh either |
| `hdf5_ugrid` | UGRID topology + values | nodes, edges, faces |
| `netcdf_ugrid` | UGRID topology + values | nodes, edges, faces |
| `geopackage` | `mesh_nodes` point features + values | node points only |

The GeoPackage writer stores node points and no connectivity, so its reader
answers with a node-only mesh rather than inventing faces. A node-only
`MeshDefinition` is already valid — the type was written that way for point
clouds and 1-D network nodes.

**S6a — the readers answer for geometry.** `IResultReader::readMesh(entry,
MeshDefinition &, message)`, defaulted to "this artifact carries none" with a
reason, because an artifact of values only is not a failure. HDF5 and NetCDF
delegate to `readUGRIDMesh`, which already exists and already handles
`start_index` and `_FillValue` padding. GeoPackage reads `mesh_nodes` in
`node_index` order through the sqlite handle it already holds.

→ verify: the mesh read back from each format equals the mesh that was
written, node for node and face for face — compared against the
`MeshDefinition` the test wrote, not against another read of the same file.
A CSV entry reports none, with a reason naming the format rather than
looking like an error.

**S6b — the item carries it.** `RecordedItem` gains
`IPolyhedralSurfaceComponentDataItem` when its reader answers with a mesh:
the entry's `location` chooses the entity (`node` → Vertex, `edge` → Edge,
`face` → Cell), and the surface is an adapter over the mesh. One interface
for all three cases rather than a geometry item for point clouds and a
surface for meshes — a consumer should not have to switch on how much
topology an artifact happened to record. Needs a `MeshDefinition` →
`PolyhedralSurface` conversion, which the SDK does not have and which is
worth more than this one caller.

→ verify: a reopened item's surface has the vertex, edge and patch counts
the written mesh had; the entity dimension the values attach to is the one
`location` names and the other two are null, as the existing spatiotemporal
items already promise; an entry with no mesh is still a valid item that
reads values and simply is not spatial.

**Interface impact:** `IResultReader::readMesh` is additive and defaulted —
no ABI break, no existing reader forced to change. `RecordedItem` is
internal to `resultsmodelcomponent.cpp`.

**Result — `db8a0b1` (S6a) and `b4ee032` (S6b).** 6 new tests in
`test_resultsreopen.cpp` (16 total in that suite), **12/12 mutations bite**
(`verification/s6/falsify_readmesh.sh`). Suite at its baseline: 245/249
serially, the four failures being the known NetCDF-writer ones (§6's
duplicate in-process netcdf/hdf5 instance on macOS).

Three of those tests exist because a mutation survived first:

- a GeoPackage point carrying an **envelope**. The SDK's writer stores none,
  so the code that steps over one was untested until a test wrote such a row.
  Without it the reader takes the first eight bytes of the bounding box as
  the coordinate.
- an **empty `mesh_nodes` table** reported as a mesh — the worst of both, a
  consumer that draws nothing and is told nothing was wrong.
- a face **ring left open**. Every patch still counts as a patch and the map
  still looks like a mesh, until you notice each cell is missing a side.

The fixture mesh gained a face for the same reason: with nodes alone, every
connectivity assertion compared zero against zero.

**Follow-up — `8cffdce`.** `PolyhedralSurfaceAdapter` rebuilt its mesh view
from the surface's vertices, so a surface with faces and edges reported a view
with neither — and the standard calls `meshView()` the accessor bulk consumers
*must* use. It cannot be recovered after the fact (a `PolyhedralSurface`
records polygons, not which vertex indices each used), so the caller that has
the connectivity passes it; the reopened item was dropping the mesh it had
just read, one call before it would have been kept. The fixture gained its
three edges, and the per-format geometry test covers them. 15/15 mutations.

**Judge this suite serially.** Several writer tests share one artifact file
(`io_results.gpkg`, `io_ugrid.h5`), so `ctest -j` races them against each
other and the failing set varies run to run. `ctest -j 1` is the honest
verdict; the parallel run's extra failures are that pre-existing collision,
not the change under test.

---

## 5. Testing standard

Adds to the existing suite (157 at v2.0.0-alpha.1):

- **Schema conformance:** every example document and generated manifest
  validated against the shipped JSON Schemas in CI.
- **Negative validation:** one test per diagnosable failure mode, asserting
  the message names the offending id — the pattern established by
  `ModelInitializerTest`.
- **Per-format round trips:** write → catalog → read → compare for CSV,
  HDF5, NetCDF, GeoPackage.
- **The reopen equivalence test** (Slice 3.3) is the acceptance criterion for
  the whole plan.
- **Coupling-from-storage:** a reopened run driving a live downstream
  component reproduces the all-live result.
- **File IO stays reviewable:** all test artifacts under `tests/artifacts/`,
  never temp directories (repository convention).

---

## 6. Risks and mitigations

| Risk | Mitigation |
|---|---|
| Schema drift between docs and code | `schema_version` is mandatory going forward; a version table in the SDK maps versions to validators; CI validates the examples against the shipped schema every build |
| CSV reads are O(file) per query | Build an index at `open()`; document CSV as the debug/interchange format, HDF5/NetCDF as the analysis formats |
| GeoPackage stores long-format rows, not arrays | Reader reconstructs `(time, object_index)` shape from the catalog entry; the entry records the shape at write time so nothing is inferred |
| Time coordinate conventions | The manifest records `time.units` explicitly (`julian_day` by SDK convention); readers never guess |
| Library paths in documents = code execution | D6: loading is opt-in, allow-listed, and never happens during validation |
| Large results in a UI | Lazy hyperslab reads plus an optional bounded per-item cache; never materialize a whole variable to answer a slice |
| Manifest/artifact drift (files moved or edited) | Optional sha256 per artifact; reader reports a clear mismatch rather than serving wrong numbers |

---

## 7. Out of scope (recorded, not done here)

- Checkpoint/restart (`resume` mode) — reserved in the schema, implemented
  with `ICheckpointableModelComponent` later.
- A visualization UI itself; this plan delivers the model-agnostic data
  access such a UI needs.
- Remote/object-store artifacts (S3-style URIs) — the reader registry is the
  seam where that would land.
- Migrating the Composer's existing XML compositions; a one-way converter is
  a small, separate utility once v1 is stable.

---

## 8. Interface impact summary

| Change | Kind | Repo |
|---|---|---|
| `ComponentStatus::Finished` doc clarification | documentation only | HydroCouple |
| `IOutputWriter::catalog()` | additive, defaulted | HydroCoupleSDK |
| Everything else | new SDK classes and headers | HydroCoupleSDK |

No v2.0 ABI break; ships as **v2.1.0**.

---

## 9. Follow-on program: argument bindings + provider components (2026-09-03)

Composition Spec grows to **v1.1** with per-argument `@from` bindings
(`{"@from": {"component", "output", "selector"}}` — selector reserved),
`ModelInitializer` gains staged apply (providers run to `Finished` before
consumers initialize; live `argument->initialize(*output)` through the
same path `SidecarSourceItem` uses), and new bulk argument classes
(`PolyhedralSurfaceArgument`, `RasterArgument`) serialize via `@uri`/`@ref`.
Provider components (mesh generator, comprehensive timeseries provider)
ship from the new HydroCoupleComponents repo. Full program:
`plans/PROVIDER_PIPELINE_PLAN_2026-09-03.md`.

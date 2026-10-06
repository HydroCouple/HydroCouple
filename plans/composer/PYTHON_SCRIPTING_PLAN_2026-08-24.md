# HydroCoupleComposer — Python Scripting Interface

**Status:** DRAFT (not started) · **Date:** 2026-08-24 · **Author:** Caleb Buahin
**Depends on:** Composer 2.0 plan phases A–D (see `COMPOSER_MODERNIZATION_PLAN_2026-08-24.md`)
**Builds on:** HydroCouple's existing Cython bindings (`HydroCouple/python/`)

---

## 1. What already exists (survey, 2026-08-24)

This is not a greenfield ask. HydroCouple already ships a **Cython** binding
layer, and its strategy document (`HydroCouple/docs/python_bindings_strategy.md`)
already commits to bidirectional coupling:

| Piece | State |
|---|---|
| `hydrocouple` Python package | Present — `core`, `spatial`, `temporal`, `spatiotemporal`, `abc` |
| `_hydrocouple` extension modules | Cython `.pyx`/`.pxd` per domain, built via setuptools + `Cython>=3.0` |
| Python components in C++ workflows | Committed design: subclass `hydrocouple.abc.ModelComponent` |
| C++ components driven from Python | **Implemented** — `hydrocouple.loader.load()` |
| SDK bindings | **None** — `HydroCoupleSDK/python/` does not exist |
| FVQual bindings | **Planned only** — `FVQUAL/python/` is a README + docs stub |

So the interface layer is bound; the SDK layer (compositions, workflows, run
manifests, results, meshing tools) is not, and nothing binds Composer itself.

### 1.1 A divergence this plan must own

`hydrocouple.loader.load()` loads components through an `extern "C"` factory
named **`CreateComponentInfo`** returning `IModelComponentInfo *`, opened with
`RTLD_LAZY` and **no ABI check**. Composer's A2 loader introduced a stamped
convention (`hydrocouple_component_abi_v1` + `hydrocouple_component_info_v1`,
`RTLD_NOW | RTLD_LOCAL`, stamp verified before any C++ call).

Left alone, a component's loadability would depend on which host opened it.
**Already resolved in Composer (A2):** the loader accepts both, and reports
legacy libraries as *unstamped* rather than as validated
(`ComponentLibrary::isUnstamped()`), covered by two fixture-backed tests.
The remaining work is to converge the other direction — see P1 below.

---

## 2. Goals

1. **S1 — Script the composition**: build, validate, run, pause, and inspect a
   Composition Spec v1 document from Python, headless or against a live
   Composer session.
2. **S2 — Script the results**: open a run manifest and read any recorded item
   as NumPy, without the producing model's library.
3. **S3 — Custom workflows**: orchestrate multi-run studies — parameter sweeps,
   calibration loops, ensembles — in Python, reusing the SDK's workflow engines
   rather than reimplementing scheduling.
4. **S4 — Python components as first-class citizens**: a component written in
   Python appears in Composer's palette and participates in couplings exactly
   as a compiled one does.
5. **S5 — Script the visualization**: drive Composer's map/3D/plot views from
   Python — load layers, theme a variable, step time, export images — so a
   figure is reproducible from a script.
6. **S6 — In-app console**: an embedded Python REPL inside Composer, operating
   on the live session's document and views.

### Non-goals

- Rewriting the GUI in Python, or a Python-authored plugin UI framework.
- Binding Qt itself (PySide/PyQt are not dependencies; see D-P3).
- A remote/RPC protocol — in-process only for now (revisit if headless cloud
  workers need it).

---

## 3. Design decisions

**D-P1 — Cython throughout, matching HydroCouple.** The SDK and Composer
bindings use Cython, not pybind11, because the interface layer already is
Cython: sharing `.pxd` declarations lets `hydrocouplesdk` and `composer`
extension types accept and return the *same* C-level `cdef class` wrappers as
`hydrocouple` without converting at each boundary. Mixing binding technologies
would force type-erased round-trips at exactly the hot path (`getValuesInto`).
*Rejected:* pybind11/nanobind — better ergonomics in isolation, wrong choice
against an established Cython layer.

**D-P2 — Three packages, layered like the C++ they wrap.**
`hydrocouple` (exists) → `hydrocouplesdk` (new) → `hydrocouplecomposer` (new).
Each is independently installable: a headless cluster job needs only the first
two and must never pull in Qt. Composer's own package is the only one that
imports GUI symbols.
*Rejected:* one mega-package — it would make Qt a transitive dependency of
every headless run.

**D-P3 — The GUI is scripted through a command façade, not by binding widgets.**
`hydrocouplecomposer` binds a narrow `ScriptingApi` C++ class — open document,
add component, connect, run, add layer, theme variable, set time, export
image — not `QWidget` trees. Every scripted mutation goes through the same
`QUndoStack` commands the UI uses (plan D8), so a script and a user editing the
same document stay consistent and a scripted change is undoable.
*Rejected:* exposing widgets via PySide — it would couple scripts to widget
layout, break on every UI refactor, and add a second Qt binding to the process.

**D-P4 — Scripts run on a worker thread; the façade marshals.** Python calls
originate off the GUI thread and reach Qt through the A3 bridge, with the GIL
released around blocking C++ (`with nogil`). The embedded console (S6) must
never freeze the UI mid-run.

**D-P5 — NumPy is the results boundary.** Recorded items are surfaced as
zero-copy NumPy arrays over the SDK's `getValuesInto()` hyperslab reads, with
the buffer protocol carrying shape/stride. No per-format readers in Python —
same rule as plan D6.

**D-P6 — A Python component is loaded like any other component.** Rather than a
separate plugin mechanism, a small C++ shim component (built once, shipped with
Composer) exports the stamped entry points and delegates every interface call
to a Python object named by its arguments. Composer's registry then needs no
Python-specific code path, and Python components gain run manifests, adapted
outputs, and workflow participation for free.

---

## 4. Work plan

Phase letters continue the Composer plan; `P` items are this program.

### P0 — Convergence and groundwork *(can start now; independent of the GUI)*

- **P1. Unify the component-loading convention.** Add the stamped entry points
  to HydroCouple's Cython loader (`load_component` gaining a stamp check and an
  explicit `allow_unstamped=False`), keeping `CreateComponentInfo` as a
  documented fallback. Upstream Composer's `componentabi.h` into the SDK so all
  three hosts share one header.
  → verify: the same fixture libraries used by Composer's loader tests
  (`testcomponent`, `legacycomponent`, `badabicomponent`) load identically from
  Python; the bad-ABI library is refused with a diagnostic, not a segfault.
- **P2. `hydrocouplesdk` Python package skeleton.** scikit-build-core + CMake
  (the SDK is a CMake library; setuptools would have to re-derive its link
  line), `.pxd` declarations for the SDK types, CI wheel job.
  → verify: `import hydrocouplesdk` on a clean venv on all three platforms;
  version matches the linked library.

### P1 — Compositions and runs (serves S1, S3)

- **P3. Composition documents.** Bind `CompositionSpec`/`ModelInitializer`:
  load, validate against the installed JSON Schemas, mutate, save. Pythonic
  surface (`doc.components["channel"].arguments["roughness"] = [...]`) over the
  JSON object model.
  → verify: round-trip of the SDK's `serial_coupling` composition is
  byte-identical; an invalid document raises with the schema's own message.
- **P4. Component resolution and execution.** Bind the registry/loader (P1) and
  the workflow engines; expose `run(document, progress=callback)` with
  cooperative pause/stop and the `errors()` queue surfaced as exceptions.
  → verify: the `serial_coupling` example runs from Python and produces a run
  manifest byte-comparable with the C++ example's.
- **P5. Study orchestration.** Thin helpers over P3/P4 for the actual use case:
  parameter sweeps, ensembles, and calibration loops that vary arguments,
  run, and collect results — with runs dispatched in parallel processes.
  → verify: a 20-member sweep over the example composition produces 20 valid
  manifests; results reduce to one table.

### P2 — Results and visualization (serves S2, S5)

- **P6. Results access.** Bind `RunManifest`/`ResultsModelComponent`/readers;
  catalog browsing and NumPy hyperslab reads (D-P5); an xarray adapter for
  UGRID/time-series items where xarray is installed (optional dependency).
  → verify: values read from Python equal the C++ `getValuesInto()` reads of
  the same hyperslabs, bitwise, with the model library absent.
- **P7. Visualization façade.** Bind `ScriptingApi`'s view surface: open a
  manifest as layers, theme a variable with a classification, set the time
  index, position the 2D/3D camera, export PNG/SVG.
  → verify: a script reproduces a reference figure hash-stably offscreen;
  the same calls work against a live GUI session.

### P3 — Python components and the console (serves S4, S6)

- **P8. Python component shim.** The delegating C++ component of D-P6 plus
  `hydrocouple.abc` subclassing docs; arguments carry the module/class to load.
  → verify: a Python component couples to a compiled one in both directions
  and its outputs land in the run manifest like any other component's.
- **P9. Embedded console.** A dockable REPL in Composer bound to the live
  session (D-P3/D-P4), with history, completion, and a "script this action"
  affordance that emits the API call for what the user just did in the UI.
  → verify: editing the document from the console updates canvas and map
  immediately and is undoable with Ctrl+Z; a long script leaves the UI
  responsive and is cancellable.

---

## 5. Milestones

| Milestone | Contents | Capability unlocked |
|---|---|---|
| **MP0** | P1–P2 | One loading convention family-wide; `hydrocouplesdk` importable |
| **MP1** | P3–P5 | Headless scripted compositions, sweeps and calibration — no GUI needed |
| **MP2** | P6–P7 | Scripted results analysis and reproducible figures |
| **MP3** | P8–P9 | Python components as peers; in-app console |

MP0/MP1 deliver value without any Composer GUI work and can proceed in
parallel with Composer phases B–C. MP2's P7 needs Composer C/D; P6 does not.

---

## 6. Risks

| Risk | Mitigation |
|---|---|
| **Two loading conventions persist** and components become host-specific | P1 is first for exactly this reason; Composer already accepts both, so the fallback is the compatibility floor, not the target |
| **Qt leaking into headless installs** | Package split (D-P2) is enforced by an import test in CI: importing `hydrocouplesdk` must not import Qt |
| **GIL stalls the GUI** during long scripted runs | D-P4: worker thread + `nogil` around blocking calls; P9's verify explicitly tests responsiveness and cancellation |
| **Cython maintenance burden** as SDK headers churn | `.pxd` declarations checked in CI against the headers — HydroCouple's CI already does this (`ci: … check .pxd declarations too`) |
| **Binding surface sprawl** | Bind the façade, not the class tree (D-P3); everything the GUI can do is expressed as a command, and commands are already enumerated by the undo stack |
| **Python components' exceptions crossing into C++** | The D-P6 shim translates them into the component's `errors()` queue and a failed status, never lets them unwind through C++ |

---

## 7. Open questions

1. **Package naming** — `hydrocouplecomposer` vs a shorter `composer`, and
   whether the SDK package is `hydrocouplesdk` or `hydrocouple.sdk`
   (namespace package under the existing one).
2. **xarray as optional or required** for P6's results adapter.
3. **Console library** — plain `code.InteractiveConsole` in a dock, or
   Jupyter-kernel integration (richer, much heavier dependency).
4. **Does P1's stamp check belong in the SDK's C++** (a shared
   `ComponentLoader` all three hosts call) rather than being reimplemented in
   Cython? This is the same question as Composer plan Q2 and should get one
   answer for both.

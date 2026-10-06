# FVQUAL Python Bindings — Implementation Plan

- **Date:** 2026-08-23
- **Status:** PY0 delivered 2026-08-23; PY1–PY5 planned
- **Parent:** `FVQUAL_FEASIBILITY_AND_DEVELOPMENT_PLAN_2026-08-23.md` (decision D-F15)
- **Template:** `openswmm.engine/python/` — the architecture below deliberately mirrors it, and the deviations are individually justified.
- **Conventions:** decision ids `D-PY#`, phases `PY0–PY5` with verify criteria, ⬜/✅ status.

---

## 1. Goal

A Python package `fvqual` that can orchestrate the entire engine — build or load
a mesh, configure sigma layering and species, run the coupled
barotropic/baroclinic simulation with scripted control over every step, read and
force state mid-run, and write UGRID results — with the same completeness the
`openswmm` package has over its engine. Python is how models get calibrated,
batch-run, coupled to optimizers, and taught; an engine without bindings is an
engine with one user.

A worked target, to make "orchestrate" concrete — the internal-seiche benchmark
as a user would script it:

```python
import fvqual
import numpy as np
from datetime import timedelta

mesh = fvqual.Mesh.channel(columns=40, length=1000.0, width=50.0, bed=-20.0)
# or: fvqual.Mesh.from_ugrid("degray.nc")

sim = fvqual.Simulation(
    mesh,
    sigma=fvqual.Sigma(layers=20),
    surface=0.0,
)

temperature = sim.species.add_temperature(initial=10.0)
sim.options.manning_n = 0.0
sim.options.advection_scheme = "muscl_fct"

# Tilted two-layer initial condition, set through a zero-copy state view.
T = sim.state.species(temperature)          # shape (columns, layers), no copy
z = sim.mesh.layer_centres()                # same shape
x = sim.mesh.column_x()[:, None]
T[...] = np.where(z > -5.0 + 1.5 * (x / 1000.0 - 0.5) * 2.0, 25.0, 5.0)

with sim:                                   # initialize; teardown guaranteed
    for elapsed in sim.steps(until=timedelta(seconds=12000)):
        if sim.diagnostics.surface_flux_residual > 1e-8:
            raise RuntimeError("mode splitting drifted")
    sim.write_ugrid("seiche.nc", species=[temperature], every=timedelta(seconds=60))
```

## 2. Architecture — three layers, C API in the middle

```
Python user code / notebooks / optimizers / (future) MCP server
        │
   fvqual (Cython package)          python/fvqual/*.pyx
        │  direct C calls, nogil on the hot path
   FVQUAL C API                     include/fvqual/capi/*.h   ← NEW LAYER
        │  extern "C", opaque handles, error codes, C89 ABI
   libfvqual-core (C++20)           the existing engine
```

**D-PY1 — Python binds a C API, never the C++ directly.** The obvious
alternative is pybind11/nanobind straight onto the C++ classes, and it is
rejected deliberately: (a) the C ABI is a stability firewall — C++ headers can
be refactored freely while `fvqual_*.h` stays fixed; (b) the C API is a
deliverable in its own right (MATLAB, R, Julia, Excel, and the eventual MCP
server all consume the same handles); (c) it is the openswmm architecture, and
diverging would mean maintaining two binding philosophies across sibling
projects. The cost — writing the C layer — is work Phase 8's HydroCouple
component wrapper partially needs anyway.

**D-PY2 — Cython, not pybind11.** Consistency with `openswmm` is worth more
than any marginal ergonomic difference: the team's binding idioms, the
`CythonHelpers.cmake` / `PatchNumpyPxd.cmake` machinery, the `nogil` patterns,
and the shipped-`.pxd` story (downstream Cython can `cimport fvqual`) all
transfer verbatim. Copy those two CMake helpers from openswmm rather than
reinventing them — the numpy-2.x `.pxd` patch and the `python -m cython`
invocation each encode a debugged pitfall.

## 3. The C API surface (`include/fvqual/capi/`)

Conventions, all inherited from `openswmm/engine/`:

- Opaque handles: `FVQ_Mesh`, `FVQ_Simulation`, as pointers to *incomplete
  struct types* rather than `void*` — equally opaque, but the compiler still
  type-checks them, so passing a mesh where a simulation belongs is a
  diagnostic instead of a crash. A small improvement on the openswmm
  convention, adopted in PY0. Every call is instance-scoped: no global state,
  so concurrent simulations in threads work from day one.
- Integer `FVQ_ERR_*` return codes; `fvq_error_message(code)` for static text;
  `fvq_last_error()` for the detail string the C++ layer's `std::string
  message` out-params already produce. **Thread-local, not per-handle** — a
  rejected mesh fails before any handle exists, and needs somewhere to put its
  explanation.
- C89-compatible declarations at the boundary; `FVQUAL_CAPI` export macro.
- `fvq_sim_advance` and `fvq_sim_step` documented GIL-safe (no Python, no
  callbacks unless registered).
- SI units everywhere, seconds for time — conversion to `datetime`/`timedelta`
  happens in Python (D-PY5). No unit-system toggle exists to bind, one of the
  places FVQUAL is deliberately simpler than SWMM (D-F8).

| Header | Contents |
|---|---|
| `fvqual_engine.h` | Master include; version macros; error codes and messages |
| `fvqual_mesh.h` | Mesh from arrays (nodes, CSR faces, bed), from UGRID file, from generators (channel strip); geometry queries (counts, areas, centroids, edges, Haney report) |
| `fvqual_sigma.h` | Sigma spec (uniform/stretched/explicit), resolve, layer geometry queries |
| `fvqual_species.h` | Register species / temperature / age, query registry, kinds, units |
| `fvqual_options.h` | Get/set every `InternalModeOptions` + `BarotropicOptions` field, keyed getters/setters plus typed shortcuts; advection-scheme enum |
| `fvqual_sim.h` | Create (mesh + sigma + surface), initialize, step, advance, stable_dt, time, destroy |
| `fvqual_state.h` | **Pointer-lending accessors** (see D-PY3): `fvq_state_species_ptr`, `fvq_state_eta_ptr`, `fvq_state_velocity_ptrs`, …, each returning base pointer + extent + a generation counter; bulk copy-in setters with validation |
| `fvqual_diagnostics.h` | The `InternalModeDiagnostics` / `BarotropicDiagnostics` fields; per-species mass totals |
| `fvqual_forcing.h` | Wind stress, surface heat flux, per-column Manning — settable mid-run |
| `fvqual_output.h` | Register a UGRID writer on a simulation (fields, cadence), flush/close; snapshot capture |
| `fvqual_forcing.h` (cont.) | **Sources and outlet works — settable mid-run.** Register/query inflows and withdrawals; weirs, orifices, pipes, pumps; multi-port outlet groups; internal transfers |

### Runtime forcing and operational control (confirmed 2026-08-23)

Scripted control of forcing and structures **while the simulation runs** is a
first-class requirement, not an afterthought: the point of an embedded engine is
to close a loop around it — operate a gate from a rule, drive a release from a
downstream target, replay an observed schedule, or wrap the whole thing in an
optimiser. Phase 4 has now built the engine side, and every lever it exposes is
a plain setter designed to be called between steps:

| C++ (delivered) | Purpose |
|---|---|
| `SourceSet::setDischarge` | Inflow / withdrawal rate on a schedule |
| `StructureSet::setOpening` | Gate position in [0, 1] — the operator's actual lever |
| `StructureSet::setTailwater` | Downstream stage a structure discharges against |
| `StructureSet::setSchedule` / `clearSchedule` | Switch a structure between a scheduled release (rating becomes the capacity limit) and its free rating curve |
| `OutletGroupSet::setDemand` / `setTarget` | Total release and target release temperature for a multi-port group |
| `TransferSet::setDischarge` | Rate of an internal transfer between columns |

with the matching readers — `discharge`, `isRunning`, `deliveredRelease`,
`achievedTemperature`, `deliveredTemperature`, `totalDischarge` — so a control
script can see the consequence of what it just did before deciding the next
step. Nothing here needs new engine work; PY-F only has to surface it.

The one constraint worth stating plainly: these are **between-step** calls.
`advance()` subcycles the barotropic mode internally and re-rates head-driven
structures on every substep (D-F16), so mutating a structure from inside a
step-end callback that fires mid-`advance()` is undefined for the same reason
writing through a state view mid-step is. Drive the loop with `sim.steps()` and
set forcing at the top of each iteration.

Deliberately absent for now: hotstart (no state-serialization story yet — track
as PY-future) and kinetics (Phase 6; the `.rxn` loader will get
`fvqual_reactions.h` when it exists).

## 4. Python package design (`python/fvqual/`)

Layout mirrors openswmm's one-module-per-header rule:

```
python/
├── pyproject.toml            scikit-build-core; [tool.cibuildwheel]; mypy; pytest
├── CMakeLists.txt            find_package(FVQUAL) or add_subdirectory(..)
├── cmake/                    CythonHelpers.cmake, PatchNumpyPxd.cmake (copied)
├── fvqual/
│   ├── __init__.py           public re-exports; DLL dir shims on Windows
│   ├── _common.pxd           all cdef extern blocks; _check(); _resolve helpers
│   ├── _mesh.pyx             Mesh, Sigma
│   ├── _simulation.pyx       Simulation, steps()/until()/run(), context manager
│   ├── _state.pyx            StateView: zero-copy numpy views + setters
│   ├── _species.pyx          SpeciesRegistry view
│   ├── _options.pyx          Options (MutableMapping + typed shortcuts)
│   ├── _diagnostics.pyx      Diagnostics snapshot
│   ├── _output.pyx           UGRID output registration
│   ├── _enums.py             AdvectionScheme, SpeciesKind, ErrorCode (pure Python)
│   ├── _exceptions.py        exception hierarchy (pure Python)
│   └── *.pyi                 hand-written stubs, mypy-gated in CI
└── tests/                    pytest; mirrors the C++ gate structure
```

**Lifecycle.** `Simulation.__enter__` initializes; `__exit__` finalizes writers
and destroys, swallowing engine errors stage-by-stage so teardown always
completes (openswmm's `Solver.__exit__` pattern verbatim). `steps()` is a
generator yielding elapsed `timedelta`; `until(t)` advances to a target;
`run()` for one-shot; step-end and progress callbacks via C trampolines
(`noexcept with gil`), enabling live plotting and early termination from
Python.

**Errors (D-PY4).** Every nonzero return passes through `cdef inline _check()`
→ `raise_for_code()`. Exceptions dual-inherit so stdlib handlers work:
`MeshError(FvqualError, ValueError)`, `LifecycleError(FvqualError,
RuntimeError)`, `DryColumnError(FvqualError, RuntimeError)`,
`FileError(FvqualError, IOError)`, `StaleViewError(FvqualError,
RuntimeError)`. Each carries `.code` and `.code_enum`.

**State access (D-PY3) — true zero-copy, an intentional improvement on the
template.** openswmm's plan calls its bulk accessors "zero-copy," but the
recon found the shipped code pre-allocates a numpy array and has C fill it —
one copy per call. That was the right call *there*: SWMM's engine reallocates.
FVQUAL's state stores are allocation-stable by design (D-F7, made for Kokkos
mirroring), which makes the real thing safe here:

- Read/write access: `np.asarray(<double[:n]> ptr)` over the lent pointer,
  with the `Simulation` object as the numpy base so the buffer cannot outlive
  the engine. Shape `(columns, layers)` views come free from the
  `cell = column*layers + k` layout — the indexing decision made in Phase 1
  pays off again here.
- Staleness guard: the C API returns a generation counter with every pointer;
  re-initialization bumps it, and a guarded view raises `StaleViewError`
  rather than reading freed memory. (Stepping does *not* bump it — that is
  what allocation-stable buys.)
- Writes through views are legitimate for initial conditions and forcing
  (exactly what the §1 example does) but the documentation must be blunt that
  mid-step mutation from a callback is undefined.
- Copying bulk getters (`state.temperature_copy()`) exist alongside, for users
  who want a snapshot semantics.

**Time (D-PY5).** The C boundary speaks seconds (`double`); Python speaks
`datetime.timedelta` (and `datetime` once a reference date exists in the
options). No decimal-day conversion layer needed — another simplification over
SWMM inherited from being SI/seconds-native.

## 5. Packaging (D-PY6, D-PY7)

**Backend:** scikit-build-core ≥0.11, CMake ≥3.24, Cython ≥3.0.12, python
≥3.10, editable installs `redirect`+`rebuild=false` (openswmm documents *why*:
auto-rebuild breaks under pip build isolation and full-engine rebuilds per
import are unusable).

**The wheel bundles the whole native closure** — `libfvqual-core` *and*
`libHydroCoupleSDK` + `libHydroCoupleTools`, since the SDK is on no package
registry. `$ORIGIN`/`@loader_path` RPATHs on POSIX, `os.add_dll_directory` +
delvewheel on Windows, exactly the openswmm mechanism. The python build either
`find_package(FVQUAL)` from a prefix (CI hot path) or `add_subdirectory(..)`
with `FVQUAL_SDK_SOURCE_DIR` pointing at a sibling SDK checkout — CI already
checks out both repos for the C++ workflow, so the wheel job reuses that.

**cibuildwheel, inheriting `CIBUILDWHEEL_REVERT_PLAN.md` wholesale.** That
document is a paid-for education: the manylinux container cannot see the
host's `$VCPKG_ROOT` (bootstrap vcpkg *inside* the container, or avoid vcpkg —
FVQUAL's `no-deps` configuration needs neither vcpkg nor NetCDF, so the *base
wheel builds dependency-free*, sidestepping openswmm's hardest problem);
manylinux_2_28 baseline; native aarch64 runners, no QEMU; pin the action
version; cp310–cp313; run pytest inside the built wheel as the gate. NetCDF
support ships as a wheel *feature* decision deferred to PY5 — options are
vendoring netcdf-c via the in-container bootstrap or making UGRID IO
`import`-guarded like openswmm's `HAS_2D`.

**Base wheel stays Kokkos-free**, mirroring openswmm's companion-wheel pattern
(`openswmm-gpu-omp`): when Phase 7 lands, GPU backends arrive as a separate
`fvqual-gpu-*` wheel discovered by the package shim, never as a base-wheel
dependency.

## 6. Testing & typing

- pytest under `python/tests/`, structured like the C++ gates: a lifecycle/
  concurrency group (two simulations in two threads, `nogil` proven), a state-
  view group (zero-copy identity: write through view, read through C API;
  staleness raises), and **physics parity as the keystone gate** — the
  internal-seiche and lock-exchange benchmarks scripted in Python must
  reproduce the `fvqual-verify` numbers to tight tolerance. That single test
  transitively certifies mesh construction, options, stepping, and state
  access, and pins the bindings to the engine's verified behaviour.
- Wheel gate: cibuildwheel `test-command` runs the pytest suite inside the
  repaired wheel on every platform.
- `.pyi` stubs hand-written and mypy-gated in CI (openswmm's typing workflow),
  strict on `_enums`/`_exceptions`.
- The C API gets its own thin C test (compiled as C, not C++) asserting the
  headers are genuinely C-compilable — the ABI firewall is only real if CI
  enforces it.

## 7. Phases

### PY0 — C API foundation ✅ (delivered 2026-08-23)

`fvqual_engine.h` (error model, version, export macro), `fvqual_mesh.h`,
`fvqual_sigma.h`, `fvqual_options.h`, `fvqual_sim.h`, implemented over the C++
engine in `src/capi/`. Exported from `fvqual_core` rather than a separate
library — one fewer artefact to ship, and the `FVQUAL_CAPI_BUILD` define
carries the Windows dllexport/dllimport switch.

Design points settled here, which PY1–PY5 inherit:

- **Handles are pointers to incomplete struct types**, not `void*`. Opaque to
  callers, still type-checked by the compiler — strictly better than the
  openswmm convention at no cost.
- **Simulations copy the mesh they are given.** Destroying a mesh mid-run is
  therefore harmless. Lifetime coupling between handles is the one error class
  a binding cannot recover from, and a few arrays copied once removes it. The
  C test asserts this explicitly.
- **Two-phase lifecycle** — `create → add species → set surface → initialize →
  step`. The split is forced by the state layout: species-major over cells has
  to be known before the first byte is reserved, and that is exactly what buys
  the allocation-stable arrays PY3's zero-copy views depend on.
- **Thread-local error detail**, because failures happen before a handle
  exists — a rejected mesh has nowhere else to put its explanation.
- **Every entry point is exception-guarded**; an exception crossing into C is
  undefined behaviour.
- `FVQ_ERR_DRY_COLUMN` is its own code, so a binding can say "dry column"
  rather than "mesh error".

**Verify ✅:** `tests/capi/test_capi.c` — 50 checks, written in C and compiled
by a C compiler — covers the error/version surface, null-handle safety, mesh
construction and bed editing, lifecycle ordering (stepping before initialize,
registering after it, double initialize, duplicate temperature all refused),
dry-column rejection, and a barotropic seiche gate: **volume drift 1.2e-15,
continuity residual 5.4e-16** over a quarter period.

Plus `c89_header_check.c`, compiled `-std=c89 -pedantic-errors`, which exists
so the ABI promise is enforced rather than merely stated. Confirmed to have
teeth: injecting a `//` comment into a public header fails the build.

### PY1 — Package skeleton + mesh/species/options bindings ⬜ (~2 weeks)
scikit-build-core project, copied CMake helpers, `_common.pxd`, `Mesh`,
`Sigma`, `SpeciesRegistry`, `Options`, exceptions, enums.
**Verify:** `pip install -e .` on the three platforms; mesh round-trip
(arrays in → geometry out) matches C++ values exactly; `pytest` green.

### PY2 — Simulation lifecycle + stepping ⬜ (~2 weeks)
`Simulation`, context manager, `steps()`/`until()`/`run()`, `nogil` advance,
step-end + progress callbacks, diagnostics.
**Verify:** concurrency test (two engines, two threads, both correct);
callback-driven early stop; teardown-after-error leaves no leak (ASan job).

### PY3 — State views ⬜ (~1–2 weeks)
Zero-copy views with generation guard, `(columns, layers)` reshape helpers,
copying getters, validated bulk setters, forcing setters.
**Verify:** view-identity test (write via numpy, read via C API, byte-equal);
`StaleViewError` on re-init; view survives 1000 steps without invalidation.

### PY4 — Physics parity + output ⬜ (~2 weeks)
UGRID output registration from Python; the §1 seiche script and a lock-exchange
script as shipped examples; parity gate against `fvqual-verify` outputs.
**Verify:** Python-scripted seiche period and lock-exchange front match the
C++ verification data within 0.1%; output `.nc` passes ugrid-checks.

### PY-F — Runtime forcing and operational control ⬜ (~2 weeks)
`fvqual_forcing.h` grown to cover the Phase 4 engine: register and query
sources, structures, outlet groups, and transfers, and drive every runtime
setter listed in §3 between steps. Python side: `Structures`, `OutletGroups`,
and `Transfers` collection objects hanging off `Simulation`, each item a small
proxy with properties (`opening`, `tailwater`, `schedule`, `demand`, `target`)
that write straight through to the engine.
**Verify:** a scripted gate-closure run reproduces the C++
`Structures.GateClosureStopsTheRelease` gate from Python; a control script that
adjusts a multi-port group's target each step holds the achieved release
temperature to a prescribed trajectory; a rule-based operation (close the gate
when the level falls past a threshold) matches the same rule expressed in C++.

### PY5 — Wheels, typing, docs ⬜ (~2–3 weeks)
cibuildwheel matrix (base wheel dependency-free; NetCDF feature decision),
`.pyi` + mypy CI, README + two notebook examples (run-and-plot seiche;
mesh-from-bathymetry workflow).
**Verify:** wheels build and self-test on Linux x86_64/aarch64, macOS
arm64/x86_64, Windows; `pip install fvqual` from a local index runs the seiche
example on a clean machine.

Total: **~12–14 weeks**, parallelizable against Phase 4/6 engine work after
PY0 fixes the C API conventions. Sequencing note: PY0 should land *before*
Phase 6 kinetics, so the kinetics C API grows inside an established convention
rather than retrofitting one.

## 8. Risks

| # | Risk | Mitigation |
|---|---|---|
| P1 | C API drifts from C++ as engine phases land | C API additions are part of each engine phase's definition-of-done from PY0 onward; parity pytest in CI catches silent breaks |
| P2 | Zero-copy views dangle after misuse | Generation guard + numpy base-object keepalive; ASan CI job; documented undefined-behaviour boundary |
| P3 | Wheel bloat / SDK bundling friction | `no-deps` base build keeps the closure to 3 libraries; measure wheel size in CI with a budget |
| P4 | numpy 2.x `.pxd` hack breaks again | The patch is copied, not linked; pin numpy build floor; openswmm hits it first and the fix transfers |
| P5 | NetCDF-in-wheel complexity | Deferred decision with two known-good fallbacks (import-guarded feature, or in-container vcpkg per openswmm) |
| P6 | Binding a moving target (Phase 4/6 not built) | Headers grow additively with the engine; nothing binds what does not exist |

## 9. Decision log

| Id | Decision | Rationale |
|---|---|---|
| D-PY1 | Bind a new C API layer, never C++ directly; pybind11 rejected | ABI firewall; multi-language deliverable; matches openswmm |
| D-PY2 | Cython + scikit-build-core; copy openswmm's CMake helpers | Sibling-project consistency; `nogil` + shipped-`.pxd` story; the helpers encode debugged pitfalls |
| D-PY3 | True zero-copy state views with generation guard; copies alongside | Allocation-stable stores (D-F7) make it safe here where it was not in SWMM; big win for scripted ICs/forcing |
| D-PY4 | Dual-inheritance exception hierarchy over integer codes | stdlib-idiomatic handling without losing engine codes; openswmm pattern |
| D-PY5 | Seconds/SI at the C boundary; `timedelta`/`datetime` in Python only | FVQUAL is SI-native (D-F8); no decimal-day legacy to carry |
| D-PY6 | Wheel bundles fvqual-core + HydroCoupleSDK libs; base wheel Kokkos-free with future GPU companion wheels | SDK has no registry presence; mirrors openswmm's proven RPATH/delvewheel + companion-wheel mechanics |
| D-PY7 | cibuildwheel per `CIBUILDWHEEL_REVERT_PLAN.md` lessons; base wheel built dependency-free | Inherit the paid-for education; FVQUAL's `no-deps` config sidesteps the vcpkg-in-container problem entirely for the base wheel |
| D-PY8 | MCP server is out of scope, planned as a sibling repo layering on this package | openswmm.mcp precedent: FastMCP → `asyncio.to_thread` → these bindings; nothing here needs to change for it |

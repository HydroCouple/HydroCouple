# Python Bindings — v2.0.0 Update Plan

**Baseline:** `python/` as of the v2.0.0 interface rework (headers: typed BufferDescriptor data plane, no variant, no code in interface headers, `hydrocoupledistributed.h`, `hydrocouplehelpers.h`).
**Date:** 2026-08-22

## What exists today

The bindings have a sound two-layer architecture worth keeping:

| Layer | Contents | State |
|---|---|---|
| `hydrocouple/` (pure Python) | ABC mirrors of the C++ interfaces — `core.py` (1,996 lines), `spatial.py` (2,881), `temporal.py`, `spatiotemporal.py`, `loader.py` | Mirrors the **pre-v2** standard: `HydroCoupleVariant` union (with the `std::any` slot), variant get/set overloads, `mpi*` methods, UI methods on core interfaces, no Checkpointing/YAML, no DataKind/BufferDescriptor/Capability/ErrorEntry, no distributed or helpers modules |
| `_hydrocouple/` (Cython) | `_variant`, `_core` (dlopen loader + two-way bridge), `_signal`, `_temporal`, `_spatial`, `_spatiotemporal`; C++ bridge headers `py_component_bridge.h` (Python object ⇄ C++ `IModelComponent*`), `signal_bridge.h`, `variant_helper.h` | Same pre-v2 API in `.pxd` declarations; `_variant` and `variant_helper.h` bind a type that no longer exists |
| Build/tests | `setup.py` + `pyproject.toml` (setuptools + Cython, `-std=c++20`, numpy), pytest suite (11 files incl. a 735-line end-to-end) | Functional but ships committed build artifacts (`.so`, generated `.cpp`, `__pycache__`, `.pytest_cache`, stray `temporal.py.new`) and has no wheel/CI story |

The two-way bridge is the crown jewel: `loader.load()` brings compiled C++ components into Python, and `py_component_bridge.h` makes a Python ABC implementation visible to C++ workflows as a real `IModelComponent*`. Both directions must survive the update.

## Design decisions

**1. NumPy *is* the BufferDescriptor.** Do not mirror the raw `BufferDescriptor` struct as a Python class. The C++ descriptor (data pointer, DataKind, shape, byte strides, memory space) is precisely the information a NumPy array already carries via the buffer protocol. The Python data plane is therefore:

```python
def get_values_into(self, destination: np.ndarray, start: Sequence[int],
                    count: Sequence[int]) -> tuple[bool, str]: ...
def set_values_from(self, source: np.ndarray, start: Sequence[int],
                    count: Sequence[int]) -> tuple[bool, str]: ...
```

The Cython layer converts ndarray ⇄ `BufferDescriptor` **zero-copy** in both directions: outbound, `descriptor.data = arr.data`, `kind` from `arr.dtype`, `shape`/`stridesBytes` straight from the array; inbound (C++ calling a Python component), the bridge wraps the descriptor's memory as a writable strided ndarray with `np.frombuffer`-style construction — no element copies ever cross the boundary. Strided/pitched descriptors map to non-contiguous ndarray views for free. `MemorySpace.Device` buffers map onto the DLPack protocol (CuPy/PyTorch interop) in a later phase; the enum is bound now, host-only enforced.

**2. Mirror, don't invent.** The ABC layer stays a 1:1 reflection of the headers — same names (snake_cased per existing convention), same inheritance, same pure-abstract discipline, and mirrors of the two governing principles: no behavior in the ABCs beyond `NotImplementedError`, and nothing bound that the headers don't declare. The one sanctioned exception mirrors the C++ one: a `hydrocouple.helpers` module mirroring `hydrocouplehelpers.h`.

**3. Errors stay mirrored, raising is opt-in.** C++ returns `bool` + message; the ABCs keep that shape (as `tuple[bool, str]`) so Python and C++ components behave identically under a workflow. `helpers` provides thin raising wrappers (`get_values_or_raise`) for scripting comfort.

---

## Phase P0 — Repository hygiene (½ day)

Remove committed build products from version control: all `.so`, Cython-generated `.cpp`, `__pycache__/`, `.pytest_cache/`, and `temporal.py.new`. Add `python/.gitignore` covering them. Delete `_hydrocouple/_variant.pyx/.pxd/.cpp/.so`, `_hydrocouple/include/variant_helper.h`, and the `_variant` extension entry in `setup.py` — the type they bind no longer exists.

*Verify:* `git status` clean of artifacts after a full build; `grep -r variant python/` returns nothing outside historical docs.

## Phase P1 — ABC layer sync (`hydrocouple/`) (3–4 days)

`core.py`: delete `HydroCoupleVariant`; add `DataKind`, `MemorySpace`, `Capability` IntEnums and an `ErrorEntry` dataclass (with `Severity`); update `ComponentStatus` (insert `Checkpointing` = 10, shift the rest) and `ArgumentInputType` (insert `YAML` = 3); replace the variant get/set family on `IComponentDataItem` with `shape()`, `data_kind`, `get_values_into`, `set_values_from`; remove `dimension_length`, UI methods, and `mpi*` members; add `capabilities()`, `errors()`, `IUIProvider`, `ILicensedComponent`, `ICheckpointableModelComponent` (`save_state`/`restore_state`); metadata accessors become native types (`missing_value/default_value/min_value/max_value → float`, `IQuality.categories → list[str]`, `clone` args `dict[str, str]`); add `IArgument.serialize()`.

`temporal.py`: `times() -> np.ndarray` (float64 Julian days), `int` (Python) indexes throughout, drop overload mirrors, slim `ITimeIdBasedComponentDataItem` to `identifiers`/`identifier_dimension`, document canonical ordering (time = dim 0).

`spatial.py`: add `IMeshView` ABC returning ndarrays (`node_x/node_y/node_z`, `face_node_offsets/face_nodes` CSR, `edge_nodes`); `mesh_view` on `INetwork`/`IPolyhedralSurface`; bulk grid accessors (`node_xs/node_ys/node_zs`, `active_cells` as bool/uint8 arrays); 64-bit counts; slim all seven component-data-item ABCs to metadata + canonical-ordering docstrings.

`spatiotemporal.py`: reduce to the seven marker combinations, matching the 158-line C++ header.

New `distributed.py` mirroring `hydrocoupledistributed.h`: `IExchangeRequest`, `Endpoint` dataclass, `ITransport` (payloads are ndarrays), `IDistributedModelComponent`, `IProxyModelComponent`, `IPartitionedComponentDataItem` (`owned_global_indexes`/`virtual_global_indexes`/`virtual_owners` as int arrays, `synchronization_epoch`, `synchronize_async`).

New `helpers.py` mirroring `hydrocouplehelpers.h`: `data_kind_size`, bidirectional `DataKind ⇄ np.dtype` mapping (this replaces `DataKindOf<T>` — the natural Python spelling), `is_valid_component_status_transition` (same table), scalar/slab conveniences and raising wrappers.

*Verify:* every ABC method name/signature traceable to a header declaration (review checklist committed alongside); enum-parity tests (P4) green.

## Phase P2 — Cython bridge rework (`_hydrocouple/`) (5–7 days)

Regenerate `_core.pxd` against the v2 header: remove variant/mpi/UI declarations; declare `DataKind`, `MemorySpace`, `BufferDescriptor`, `Capability`, `ErrorEntry`, `getValuesInto`/`setValuesFrom`, `capabilities()`, `errors()`, the side interfaces, and the distributed header's types.

The marshalling core, both directions zero-copy:

- *Python driving C++* (`loader` path): wrap C++ data items so `get_values_into(arr, start, count)` builds a `BufferDescriptor` from the ndarray (dtype→`DataKind` via helpers map, byte strides passed through, `MemorySpace.Host`) and calls the virtual directly. Release the GIL around `getValuesInto`/`setValuesFrom`/`update()` so C++ components compute in parallel with Python threads.
- *C++ driving Python* (`py_component_bridge.h`): implement the new pure virtuals — `shape`, `dataKind`, `getValuesInto`/`setValuesFrom` (construct a writable strided ndarray view over the descriptor's memory, call the Python method, translate the `(bool, str)` return), `capabilities()` (translate the Python set), `errors()` (translate `ErrorEntry` lists) — and delete the removed ones. Acquire the GIL on entry, never hold it across nested C++ calls.

`signal_bridge.h`: update `IComponentDataItemValueChanged` payloads to `start()`/`count()` int64 vectors. `loader.py`: unchanged surface; add `capabilities` surfacing and document that a `Failed` status should prompt an `errors()` drain. Distributed types get *consuming* bindings only (test/wait on requests, read partition metadata as ndarray views); transport implementations are SDK territory and are explicitly out of scope — no speculative binding.

*Verify:* round-trip pointer-identity test proves zero-copy (ndarray written by C++ shares its base address with the buffer Python allocated); pitched-destination test passes through the bridge; GIL-release test shows two C++ components updating concurrently from Python threads.

## Phase P3 — Packaging and CI (2–3 days)

Keep setuptools + Cython (adequate; no need for scikit-build-core at this size) but: single-source the version from the repo's `version.h` at build time; `Cython.Build.cythonize` in `setup.py` so generated `.cpp` never needs committing; sdist rules excluding artifacts; `py.typed` marker with regenerated `.pyi` stubs for every module. Add `cibuildwheel` to the repo's CI matrix (CPython 3.10–3.13 × linux/macos/windows) building wheels against the in-repo headers and running the pytest suite in each; wheels attach to the GitHub release alongside the header tag so the Python package version always equals the interface version it binds.

*Verify:* `pip install .` from a clean clone with only a C++20 compiler present; CI wheel matrix green; `pip install hydrocouple-2.0.0-*.whl && python -c "import hydrocouple"` on all three OSes.

## Phase P4 — Test suite port and parity guarantees (3–4 days)

Port the existing 11-file pytest suite to the v2 API (the 735-line end-to-end test is the priority — it exercises both bridge directions). Add:

- **Enum-parity tests**: a small parser extracts enum names/values from the C++ headers at test time and asserts equality with the Python IntEnums — `ComponentStatus`, `ArgumentInputType`, `DataKind`, `MemorySpace`, `Capability`, and the spatial/temporal enums can then never drift again (the pre-v2 drift this plan corrects is exactly this failure mode).
- **Transition-table parity**: exhaustive 15×15 comparison of `helpers.is_valid_component_status_transition` against the C++ table (via a tiny bound shim or a generated table snapshot).
- **Data-plane contract tests**: a pure-Python `GridDataItem` mirroring `tests/test_core_types.cpp`'s reference implementation, exercised through the same cases (interior hyperslabs, strided destinations, dtype/bounds/rank rejection), plus the zero-copy and GIL tests from P2.
- **Dtype mapping tests**: every `DataKind` ⇄ `np.dtype` pair round-trips; endianness-explicit dtypes rejected cleanly.

*Verify:* full suite green on the CI matrix; coverage report shows every ABC and every bridge path exercised.

## Phase P5 — Docs and examples (2 days)

`python/README.md` rewritten for v2: quickstart for both directions (load a C++ component and exchange ndarrays; implement a Python component and hand it to a C++ workflow), the NumPy-as-BufferDescriptor contract, canonical dimension orderings, error-queue idiom. Two runnable examples under `python/examples/`: a Python time-series component consumed through the bridge, and a script driving a C++ component with hyperslab reads into matplotlib. Cross-link from the repo README and `CHANGELOG.md`.

---

## Sequencing and effort

P0 → P1 → P2 → (P3 ∥ P4) → P5; roughly **3 weeks** focused effort. P1 is pure Python and immediately useful even before the bridge lands (Python component authors code against the new ABCs); P2 is the long pole; P3/P4 parallelize once the API is stable.

Out of scope, deliberately: binding `ITransport` implementations (SDK owns transports), device-memory (DLPack/CuPy) marshalling (bind the enum, defer the plumbing until the SDK's GPU phase ships), and any Python-side convenience layer beyond the sanctioned `helpers` mirror — same no-over-abstraction rule as the headers.

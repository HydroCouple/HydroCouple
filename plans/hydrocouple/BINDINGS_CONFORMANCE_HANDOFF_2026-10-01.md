# Python bindings conformance — Mac hand-off (2026-10-01)

Follows `plans/sdk/ENFORCEMENT_HANDOFF_2026-10-01.md`, which is still
unverified on the Mac. Run that checklist first; this round only touches
`HydroCouple/python` (plus the CHANGELOG and the commit draft), so it adds
one step to it rather than replacing it.

**State of this hand-off:** built and run in a Linux container, twice, with
both compilers:

| Build | Result |
|---|---|
| GCC, `build_ext --inplace`, `pytest` | 357 passed, 2 skipped (`test_dlpack`, `test_gradients`: no torch here) |
| Clang (`CC=clang CXX=clang++`), same | 357 passed, 2 skipped |

Not run here: the macOS build (Apple Clang, libc++), and the torch/JAX
tests. Clang prints nine `-Woverloaded-virtual` warnings from the test
fixtures' `QuietIdentity` base (it overrides the property signal and hides
the other `ISignal` overloads by name); they are harmless and were already
present from `layered_test_fixtures.h`.

## 1. The problem

Every Cython wrapper is registered with `ABC.register()` as the interface it
wraps, which makes `isinstance(output, IOutput)` true. Registration checks
nothing. An audit found all 47 registered wrappers short of their ABCs —
no `time_kind` on time series, none of `validate`/`prepare`/`request_stop`/
`request_pause`/`resume`/`errors` on workflows, no `connect`/`disconnect`
anywhere, spatial wrappers without identity, navigation or raster I/O — and
each failed only at its first attribute access.

## 2. What changed (all under `HydroCouple/python` unless noted)

- **New tests**
  - `tests/test_wrapper_conformance.py` (71): each of the 70 registrations
    has every abstract member of its ABC, as a property where the ABC
    declares a property and a method where it declares a method.
  - `tests/test_abc_parity.py` (94): every ABC compared member for member
    with the pure virtuals of its C++ class in `include/*.h`; deliberate
    differences are listed with reasons (`RENAMED`, `NOT_MIRRORED`,
    `PYTHON_ONLY`).
  - `tests/test_wrapper_behaviour.py` (34): the wrappers driven against
    native fixtures — `_hydrocouple/include/binding_test_fixtures.h`,
    exposed as `_testing.BindingFixture`, with live-object counters that
    prove what is freed and when.
  - `tests/test_examples.py` (3): the three examples run as scripts (the
    third compiles `tests/cpp_component/test_component.cpp` with the first
    of `g++`/`clang++`/`c++` on `PATH`).
- **`hydrocouple/core.py`:** the `IWorkflowComponent` ABC gains `validate`,
  `prepare`, `request_stop`, `request_pause`, `resume`, `errors` — added to
  the C++ interface at ABI 4, missed by the mirror. Found by the parity test.
  A Python class implementing `IWorkflowComponent` must now define them.
- **Wrappers** (`_core`, `_temporal`, `_spatial`, `_spatiotemporal` `.pxd`/
  `.pyx`) restructured onto shared bases (`CppPropertyChangedWrapper` →
  `CppDescriptionWrapper` → `CppIdentityWrapper` →
  `CppComponentDataItemWrapper`), each subclass binding its typed pointer by
  `dynamic_cast` (new `include/interface_casts.h`). New wrappers for every
  remaining interface. New views `as_spatial`, `as_time_series`,
  `as_time_model_component`, `as_spatiotemporal`.
- **Semantics** (documented in `python/README.md`, "Working with compiled
  objects"): equality is identity of the C++ object; a child wrapper keeps
  its parent alive; C++-created objects are owned by their wrapper;
  `connect(slot)` reaches the signal the ABC documents and passes its
  event-arguments object; one `_SlotHandle` replaces six handle classes;
  mutators declared `except +`, so a C++ throw is a Python exception.
- **Fixed:** `update(required_outputs)` and `add_model_component(c, role)`
  ignored their arguments; both pure-Python examples had stopped
  instantiating at ABI 4 (`value_kind`, `states`).
- `include/signal_bridge.h` gains `connect/disconnect_property_slot_any` and
  `block_signals_any`.
- `HydroCouple/CHANGELOG.md` — new section "Python bindings: every wrapper
  implements the ABC it is registered as";
  `plans/hydrocouple/COMMIT_HYDROCOUPLE_2026-10-01.txt` — extended to cover
  it (still one HydroCouple commit: this round edits the same `.pyx` files
  as the previous one, so it cannot be split by file).

## 3. Verification checklist (adds to step 2 of the enforcement hand-off)

1. Rebuild **with `--force`** — the `.pyx`/`.pxd` changed and a stale
   generated `.cpp` would otherwise be reused:

   ```sh
   cd ~/Documents/Projects/cbuahin_github/HydroCouple/python
   python setup.py build_ext --inplace --force && python -m pytest tests
   ```

   The enforcement hand-off expected 226 passed, 3 skipped. This round adds
   202 tests (94 + 71 + 34 + 3) and changes no existing count, so
   **expect 428 passed, 3 skipped**.
2. Things only the Mac exercises:
   - libc++'s `abi::__cxa_demangle`, used by `typeName()` in
     `interface_casts.h` to turn `IArgument::validComponentDataItemTypes()`
     into readable names. `TestComponentItems::test_argument` checks a demangled
     name contains `FixtureArgument`; if libc++ names differ, that is the
     test that says so.
   - `test_examples.py::test_drive_cpp_component` compiles a shared library
     with the system compiler; if Apple's linker objects, the failure text is
     in the assertion.
   - `test_dlpack.py`/`test_gradients.py` with torch: nothing in this round
     touches the data plane they test, but they import `_core`, which was
     rewritten — they are the check that the DLPack paths still bind.
3. **Falsification** (each must turn tests red; restore from the backup and
   rebuild with `--force`):

   ```sh
   HC=~/Documents/Projects/cbuahin_github/HydroCouple/python
   cp $HC/_hydrocouple/_core.pyx /tmp/core.bak

   sed -i '' 's/(<CppPropertyChangedWrapper>child)._owner = owner/pass/' \
       $HC/_hydrocouple/_core.pyx
   # -> test_what_is_reached_through_a_created_instance_keeps_it_alive fails
   cp /tmp/core.bak $HC/_hydrocouple/_core.pyx

   sed -i '' 's/_connect(self, _STATUS, slot, _status_adapter(self, slot))/_connect(self, _STATUS, slot, None)/' \
       $HC/_hydrocouple/_core.pyx
   # -> 3 TestSignals tests fail
   cp /tmp/core.bak $HC/_hydrocouple/_core.pyx

   cp $HC/_hydrocouple/_temporal.pyx /tmp/temporal.bak
   sed -i '' 's/    def time_kind(self):/    def time_kind_x(self):/' \
       $HC/_hydrocouple/_temporal.pyx
   # -> 2 conformance + 2 behaviour tests fail
   cp /tmp/temporal.bak $HC/_hydrocouple/_temporal.pyx

   cp $HC/hydrocouple/core.py /tmp/corepy.bak      # no rebuild needed
   sed -i '' 's/    def request_pause(self) -> None:/    def request_pause_x(self) -> None:/' \
       $HC/hydrocouple/core.py
   # -> test_abc_parity[IWorkflowComponent] and the workflow conformance test fail
   cp /tmp/corepy.bak $HC/hydrocouple/core.py
   ```

   All four were run in the container with exactly these results.

## 4. Commit

Unchanged from the enforcement hand-off §3: one HydroCouple commit of the
whole working tree with `plans/hydrocouple/COMMIT_HYDROCOUPLE_2026-10-01.txt`,
after the checklist is green. Check `git status` before `add -A`: on
2026-10-01 it showed an untracked top-level `install/` (not from these
rounds — likely a CMake install prefix) that probably should not be
committed. `python/build/`, `*.so` and the generated `_hydrocouple/*.cpp`
are already ignored by `python/.gitignore`.

## 5. Not done

- A Python-implemented spatial or layered item is still not visible to C++
  as such: `PyComponentBridge` carries the data plane, not the spatial side
  interfaces.
- `_SlotHandle` is private by name. `test_signals.py` imports it; nothing
  else outside the bindings should.
- The `-Woverloaded-virtual` warnings in the test fixtures (see above).

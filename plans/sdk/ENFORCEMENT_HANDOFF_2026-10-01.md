# Enforcement round + Python vertical structure — Mac hand-off (2026-10-01)

Follows `ABI4_HANDOFF_2026-09-29.md` and the Mac verification of it
(`HydroCoupleComposer/verification/MAC_VERIFICATION_ABI4_2026-09-30.md`).
ABI 4 added declarations; this round makes the SDK read them, and brings the
Python mirror's vertical-structure interfaces up to the C++ ones.

**State of this hand-off:** built and run in a Linux container (GCC, C++20)
against the shipped headers:

| Suite | Container result |
|---|---|
| HydroCouple C++ (`ctest`) | 155/155 |
| HydroCouple Python (`build_ext --inplace`, `pytest`) | 155 passed, 2 skipped |
| HydroCoupleSDK (`USE_GDAL/NETCDF/HDF5/GEOPACKAGE=ON`, no Torch/MPI/Kokkos) | 381 passed, 1 skipped (YAML) |
| HydroCoupleComposer | **not built** (no Qt here) — nothing in it changed this round |

Not run here: the Torch, MPI and Kokkos SDK builds, the static-full build the
Composer links, the falsify scripts, and the Composer suite.

## 0. Before anything

All three repos still have the stale `.git/index.lock` from 2026-09-29/30;
`git status` shows `MM` on files that equal HEAD because of it.

```sh
for r in HydroCouple HydroCoupleSDK HydroCoupleComposer; do
  rm -f ~/Documents/Projects/cbuahin_github/$r/.git/index.lock
  git -C ~/Documents/Projects/cbuahin_github/$r reset -q
done
```

## 1. What changed

### HydroCouple (interface + Python)

- `hydrocouplehelpers.h`: **`Failed` is reachable from every status but
  `Finished`**, for components and workflows. The old tables admitted it only
  from in-flight statuses, contradicting `IProxyModelComponent` (a peer can
  die while the proxy rests at `Updated`). Lifecycle diagrams and the
  `Failed` doc in `hydrocouple.h` say the same. New C++ tests pin it.
- `hydrocouple.h` "Toolchain" convention and README now point at
  `hydrocouplecomponentabi.h` (the peer's header — not shipped here, it is
  already on the Mac with its `CMakeLists.txt` line).
- Python `helpers.py`: transition tables were **stale since ABI 2**; now
  ABI 4 for components and (new) workflows, with
  `tests/test_transition_parity.py` comparing every pair against the C++.
- Python vertical structure: `IVerticalCoordinate`, `ILayering`,
  `ICrossSection`, `ILayeredMeshComponentDataItem`,
  `ILayeredNetworkComponentDataItem`, `VerticalCoordinateKind`,
  `CrossSectionKind` (spatial.py); `ITimeLayeredMeshComponentDataItem`,
  `ITimeLayeredNetworkComponentDataItem` (spatiotemporal.py). Cython
  wrappers for all of them; `_spatial.as_layered(item)` and
  `_spatiotemporal.as_time_layered(item)` cross-cast a component's item
  (`CppOutputWrapper`, `CppInputWrapper`, `CppComponentDataItemWrapper`) to
  them through `include/layered_casts.h`. `interface_elevations` is a
  read-only zero-copy `(columns, layers + 1)` view; `evaluate()` passes the
  caller's arrays to C++ as spans with the GIL released and refuses an
  output it could only fill by copying. `data_item` added to the argument,
  input and output wrappers.
- `python/tests/test_layering.py` (36 tests): ABCs checked member-for-member
  against `hydrocouplespatial.h`; native fixtures
  (`include/layered_test_fixtures.h`, exposed as `_testing.LayeredFixture`)
  driven through the bindings.

### HydroCoupleSDK

- **Lifecycle enforcement.** `setStatus()` checks every transition; an
  illegal one is performed and reported as `Error`,
  `Diagnostic::IllegalStatusTransition` (1001), once per (from, to) pair per
  object. Codes live in the new `core/diagnosticcodes.h`.
- **Connection validator** (`component/connectionvalidator.h/.cpp`), run by
  `AbstractWorkflowComponent::validate()` over every wired link through the
  new virtual `validateConnections()`. Error = both ends declare and
  contradict (fails validation); Warning = a declaration is missing (queued,
  fails nothing). Codes 1101–1112.
- **Proxy status mirroring.** `ComponentWorker` stamps
  `kFlagStatusPresent (0x8000) | status` into every reply's flags;
  `ProxyModelComponent` mirrors `Done`/`WaitingForData`, walks the lawful
  intermediate states, and goes `Invalid` (not `Failed`) on a rejected
  validate. Old peers send 0 and read as "no status".
- `TimeSeriesOutput<T>`/`TimeSeriesInput<T>` declare `None`/`Refuse`.
- **Fixed — `UnitDimensions` out of bounds.** 9 slots for 10 dimensions:
  `PlaneAngle` was read/written past the array and dropped by `copy()`.
  **Layout change: rebuild everything that links the SDK.**
- **Fixed — `LayeredMeshInput/Output` were uninstantiable** (pre-ABI-4
  accessors, no `vectorBasis()`); `test_templatecompleteness.cpp` now
  asserts every shipped class template is concrete.
- Fixtures in `test_executionmodes`, `test_resultsreopen`, `test_io`,
  `test_distributed` make only lawful transitions and `validate()` before
  `prepare()`.

### HydroCoupleComposer

Nothing changed this round. The ABI-4 port (7 files + the verifier's fixture
fix) is still uncommitted; its message is `plans/composer/COMMIT_ABI4_PORT.txt`.

## 2. Verification checklist

1. **HydroCouple C++:** configure/build/ctest as before — expect all pass
   (the two `FailedIsReachableFromEveryStatusButFinished` tests are new).
2. **HydroCouple Python:** `cd python && python setup.py build_ext --inplace
   --force && python -m pytest tests`. Previous Mac result was 186 passed,
   3 skipped; this round adds 40 tests (2 transition parity, 36 layering,
   2 enum parity) → **expect 226 passed, 3 skipped**.
3. **HydroCoupleSDK:** rebuild every build directory you use (the
   `UnitDimensions` layout changed), then `ctest --output-on-failure`. New
   tests: `ConnectionValidatorTest.*` (31), `WorkflowConnectionTest.*` (2),
   `LifecycleEnforcementTest.*` (3),
   `TemplateCompletenessTest.EveryShippedTemplateIsConcrete`,
   `UnitDimensionsTest.HoldEveryFundamentalDimension`,
   `ProxyWorkerTest.ProxyMirrorsTheRemoteReachingDone`.
   Things that only the Mac builds exercise and that touch this round:
   - **Torch build** (`verification/g2/falsify.sh`): differentiation fixtures
     drive components through `setStatus`; look in their `errors()` for code
     1001 — any hit is a fixture making an illegal transition, not a
     failure, but worth fixing the way the container fixtures were.
   - **MPI build** (`test_mpitransport`): the proxy now reads status from
     reply flags; a worker built from an older SDK sends no flags and the
     proxy falls back to its old behaviour.
4. **HydroCoupleComposer:** rebuild against the new SDK (layout change),
   full `ctest -j1` offscreen. Expected unchanged from 64/65
   (`test_viewhandoff` predates the port). Two behavioural notes, neither a
   failure:
   - Link **Warnings** now land in the workflow's `errors()`, so
     `SimulationManager::errors()` lists them (e.g. "neither end declares a
     ValueKind" for the test component's unit-less items). A link **Error**
     would now fail a run at validate — none of the Composer fixtures
     declare contradictions.
   - `tests/fixtures/testcomponent/legacycomponent.cpp` goes `Updated →
     Done` without `Updating`; it now gets one code-1001 entry in its own
     `errors()`. Left alone deliberately: it stands for an old component.
5. **Falsification** (each must turn its tests red; restore from the backup,
   not `git checkout`, since nothing is committed yet):

   ```sh
   SDK=~/Documents/Projects/cbuahin_github/HydroCoupleSDK
   cp $SDK/src/hydrocouplesdk/component/abstractmodelcomponent.cpp /tmp/amc.bak
   sed -i '' 's/!HydroCouple::Helpers::isValidComponentStatusTransition(prev, status))/false)/' \
       $SDK/src/hydrocouplesdk/component/abstractmodelcomponent.cpp
   # build; ctest -R LifecycleEnforcement  -> 2 of 3 fail
   cp /tmp/amc.bak $SDK/src/hydrocouplesdk/component/abstractmodelcomponent.cpp

   cp $SDK/src/hydrocouplesdk/component/abstractworkflowcomponent.cpp /tmp/awc.bak
   sed -i '' 's/const std::vector<ConnectionFinding> links = validateConnections();/const std::vector<ConnectionFinding> links;/' \
       $SDK/src/hydrocouplesdk/component/abstractworkflowcomponent.cpp
   # build; WorkflowConnectionTest.* -> both fail
   cp /tmp/awc.bak $SDK/src/hydrocouplesdk/component/abstractworkflowcomponent.cpp

   HC=~/Documents/Projects/cbuahin_github/HydroCouple/python
   cp $HC/_hydrocouple/_spatial.pyx /tmp/sp.bak
   sed -i '' 's/return _f64_view(span).reshape(columns, interfaces)/return _f64_view(span).reshape(columns, interfaces).copy()/' \
       $HC/_hydrocouple/_spatial.pyx
   # build_ext --inplace --force; pytest tests/test_layering.py
   #   -> test_bulk_profile_is_a_read_only_view_of_the_producers_storage fails
   cp /tmp/sp.bak $HC/_hydrocouple/_spatial.pyx   # then build_ext --force again
   ```

   (In the container these three were run and caught; so was deleting
   `IVerticalCoordinate.geometry_epoch` from `spatial.py`, which the
   member-parity test catches.)

## 3. Commits (after the checklist is green)

In this order, each repo's whole working tree (including the peer's
`hydrocouplecomponentabi.h` + `CMakeLists.txt` line in HydroCouple, and the
ABI-4 verification record):

```sh
git -C .../HydroCouple add -A && git -C .../HydroCouple commit -F plans/hydrocouple/COMMIT_HYDROCOUPLE_2026-10-01.txt
git -C .../HydroCoupleSDK add -A && git -C .../HydroCoupleSDK commit -F ../HydroCouple/plans/sdk/COMMIT_SDK_ENFORCEMENT_2026-10-01.txt
git -C .../HydroCoupleComposer add -A && git -C .../HydroCoupleComposer commit -F ../HydroCouple/plans/composer/COMMIT_ABI4_PORT.txt
```

Check `git status` before `add -A`: the SDK's `tests/artifacts/` are
regenerated by every test run, and the Composer's `verification/` holds
build and ASan logs you may not want committed.

## 4. Not done (deliberately)

- A Python-implemented `IVerticalCoordinate`/`ICrossSection` is not visible
  to C++: the bridge (`py_data_item_bridge.h`) carries the data plane, not
  the spatial side interfaces. Python can *read* layered C++ items; C++
  cannot yet read layered Python ones.
- The existing thin spatiotemporal wrappers (`id`, `time_count`) and the
  temporal wrapper's missing `time_kind`/`time_interpolation` accessors are
  unchanged; the new time-layered wrappers expose `time_count`, `times`,
  `time_dimension` on top of the full layered surface.
- The Composer's `unitIsDimensionful()` ignores `PlaneAngle`. A radian is
  dimensionless in SI, so that is defensible, but it is now a choice the
  code should state.
- Ontology items from the 2026-09-29 review remain on hold, as asked.

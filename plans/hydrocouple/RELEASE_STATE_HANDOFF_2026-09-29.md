# Hand-off: `ICheckpointableModelComponent::releaseState` (between G2.2 and G2.3)

**Repos:** HydroCouple (`dev`) — the interface, bindings, overlays;
HydroCoupleSDK (`removing_qt`) — the reverse engine · **Date:** 2026-09-29
**Plan:** `plans/hydrocouple/DIFFERENTIABLE_INTERFACE_ALTERNATIVES_2026-09-28.md`
**Why:** the G2.2 Mac review found that the checkpoint contract had no way to
give a saved state back. The logarithmic ladder saves hundreds of states and
drops almost all of them; for a component whose token names a file, every
drop was a file left on disk, so disk grew as O(N) behind an O(log N)
ladder. Decided: add it to the interface now, before G2.3.

## What changed

**Interface (HydroCouple, `include/hydrocouple.h`)**

```cpp
[[nodiscard]] virtual bool releaseState(const std::string &token, std::string &message) = 0;
```

on `ICheckpointableModelComponent`, beside `saveState` / `restoreState`.
Whatever `saveState` set aside for the token may be reclaimed; a component
whose token *is* the state returns true; after the call the token is dead
(restoring or releasing it again is an error the component may refuse).
Standard-library-only, no new include. **`HYDROCOUPLE_ABI_VERSION` is now
3**: a pure virtual was added, so the vtable grew and every component that
implements `ICheckpointableModelComponent` must add `releaseState` and be
rebuilt. In the ecosystem that is only the test fixtures (the HydroCouple
Python reservoir and the SDK's `Reservoir`); no SDK, Composer or FVQual class
implements the interface.

**Bindings (HydroCouple `python/`)**
- `hydrocouple.core.ICheckpointableModelComponent.release_state` (abstract).
- `CppModelComponentWrapper.release_state(token)`.
- The torch/JAX overlays release a step's checkpoint when its record is
  garbage-collected (`weakref.finalize`, not at interpreter exit): for
  PyTorch when the graph is dropped, for JAX once backward has consumed it.
- The Python reservoir fixture numbers its tokens and publishes the count of
  live ones as a `live_checkpoints` result item, and refuses a dead token.

**SDK engine (`DifferentiableWorkflow`)** — every checkpoint it saves is
released exactly once, as soon as nothing can restore it: a rung the ladder
drops; a bisection temporary when its half is done; the final state
backward() returns to, once it is back; every remaining rung on
`clearRecording()` and on `finish()` (which now releases before the
components finish). The destructor does not release (components may be
gone). New metrics: `releasedTokens()`, `refusedReleases()`.

## Gates

| Where | New gates |
|---|---|
| HydroCouple C++ | `CheckpointTest.SavedStatesCanBeReleased` — the member exists with the right signature; ABI version is 3 |
| HydroCouple Python (`test_gradients.py`) | `TestCheckpointsAreReleased`: a released token is dead (restore and second release refused); PyTorch releases every step's checkpoint when the graph goes (live = 6 per component after forward, 0 after `del loss` + GC); JAX releases every step once `jax.grad` returns |
| SDK (`test_differentiation.cpp`) | `EveryCheckpointTheLadderDropsIsReleased` (after every step, tokens a component holds = rungs the workflow holds); `BackwardReleasesItsTemporariesAndTheFinalState`; `ClearingOrFinishingReleasesTheRecord`; `NoTokenIsEverReleasedTwice` (forward, two backwards, finish: no dead-token use) |

## Verified off-machine (Linux, g++ 13, lean SDK build)

| Suite | Result |
|---|---|
| HydroCouple C++ | **146 passed** |
| HydroCouple Python | **184 passed, 3 skipped** (the CUDA rows) |
| SDK `Differentiation*` | **25 passed** |
| SDK full suite | **291 passed**, 1 skipped, the same 4 lean-only `ResultsReopen` failures (pass on `macos-full`) |
| SDK `verification/g2/falsify.sh` | **29/29 caught** — 23 as before (R2, R6 retargeted at the edited code) + X1–X6: a dropped rung not released, a temporary not released, the final state not released, `clearRecording` forgetting without releasing, `finish()` not releasing, a temporary released twice |
| HydroCouple `verification/g0g1/falsify.sh` | see the commit message: 23 as before + O5 (step records never release their checkpoint) |

## Run on the Mac

The SDK reads the interface from the sibling checkout or an installed
package; if it is the package, **reinstall the interface first** — an old
install lacks `releaseState` and the SDK's test fixture will fail to
compile ("marked override but does not override").

```sh
cd HydroCouple/python && python3 setup.py build_ext --inplace --force && python3 -m pytest -q tests   # 185 + 3 skipped with the untracked version.h
cd ../ && cmake --build build-tests && ./build-tests/tests/hydrocouple_tests --gtest_brief=1                  # 146
cd ../HydroCoupleSDK && cmake --build build-macos-full --parallel
./build-macos-full/tests/HydroCoupleSDKTests --gtest_filter='Differentiation*'                               # 25
./build-macos-full/tests/HydroCoupleSDKTests --gtest_brief=1                                                 # 354 + 1 skip
BUILD=build-macos-full verification/g2/falsify.sh                                                            # 29/29
cd ../HydroCouple && verification/g0g1/falsify.sh                                                            # 24/24
```

## Next

G2.3 — the optional LibTorch layer in the SDK.

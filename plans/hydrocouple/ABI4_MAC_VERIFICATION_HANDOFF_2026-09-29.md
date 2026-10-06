# Hand-off to a Mac verifier: ABI 4 (contract-consistency round) + SDK port

**For:** an agent working on Caleb's Mac, in a terminal, with the repos at
`~/Documents/Projects/cbuahin_github/{HydroCouple,HydroCoupleSDK}`.
**Date:** 2026-09-29.

**Why this exists.** The interface round and the SDK port have been built
and run on Linux, but never on macOS. Nothing is committed. The next phase
(G2.4, `plans/hydrocouple/G2_4_STATEFUL_ADAPTERS_PLAN_2026-09-29.md`) waits
on both being verified here and committed.

**Background reading.** Read these before starting:

- `plans/sdk/ABI4_HANDOFF_2026-09-29.md`, especially its last section,
  "Built and run off-machine … and four fixes".
- `HydroCouple/CHANGELOG.md` (Unreleased).
- `HydroCoupleSDK/CHANGELOG.md` (Unreleased).

## Your job

1. Set up the `hydrocouple` conda environment (§1).
2. Run every check in §2 and compare against the expected results.
3. Fix what fails, within the rules in §3.
4. When everything is green, record the result and commit (§4).
5. End with the report in §5. Caleb pastes it back to the planning agent,
   so it must stand on its own.

## State you start from (do not reset it)

**HydroCouple, branch `dev`, HEAD `fada009`.**
- 26 modified files: the ABI 4 round.
- `verification/g0g1/falsify.sh` is modified: a check whose tests are all
  skipped now reports `SKIPPED GATE` instead of `SURVIVED`.
- Untracked:
  - `python/environment.yml` (new; §1).
  - `plans/` (never commit it).
  - `install/`, a stale install of the ABI 3 headers (see pitfall P2).

**HydroCoupleSDK, branch `removing_qt`, HEAD `f16a307`.**
- 53 modified files: the port (48), plus five files carrying four fixes.
- The fixes:
  1. `test_device.cpp`: `DeviceBackend` is ambiguous between the new
     interface enum and the SDK class; the two uses are qualified.
  2. `abstractadaptedoutput.cpp`: `addConsumer()` / `removeConsumer()` now
     set and clear the consumer's provider.
  3. `abstractadaptedoutput.cpp`: the destructor detaches the adapters
     built on top of it.
  4. `meshadapters.h`: `edgeCount()` counts the derived edges, with
     `test_spatialadapters.cpp` updated to match.
- Also modified: `AtenModelComponent::states()` and the test `Reservoir`'s
  `states()`.

**Do not revert or re-apply any of this.** It is the work under test.

## 1. The conda environment

```sh
cd ~/Documents/Projects/cbuahin_github/HydroCouple
conda env create -f python/environment.yml     # or: conda env update -f python/environment.yml --prune
conda activate hydrocouple
python -c "import sys, numpy, Cython, torch, jax; print(sys.version.split()[0], numpy.__version__, Cython.__version__, torch.__version__, jax.__version__)"
```

- If torch or jax cannot be installed for Python 3.13 on macOS arm64,
  change `python=3.13` to `python=3.12` in `environment.yml`, remove the
  environment (`conda env remove -n hydrocouple`), create it again, and
  record the change.
- Every Python command below runs inside this environment. Pass
  `PYTHON=python` to the falsifier explicitly.

## 2. Checks and expected results

The expected numbers come from the Linux run (g++ 13), except where a Mac
baseline is named.

| # | Command (from the repo root) | Expected |
|---|---|---|
| H1 | `cmake --build build-tests && ./build-tests/tests/hydrocouple_tests --gtest_brief=1` (configure with `-DHYDROCOUPLE_BUILD_TESTS=ON` if needed) | **153 passed** |
| H2 | `cd python && python setup.py build_ext --inplace --force && python -m pytest -q tests` | **186 passed, 3 skipped** (the 3 are CUDA rows) |
| H3 | `PYTHON=python verification/g0g1/falsify.sh` | **24/24 caught**, no `SKIPPED GATE` |
| S1 | `cmake --build build-macos-full --parallel && ./build-macos-full/tests/HydroCoupleSDKTests --gtest_brief=1` | Mac baseline before the port: **358 passed, 1 skipped**. The port adds or removes no tests, so expect the same. (Linux, NetCDF + HDF5 + GeoPackage, no tools: 343/343.) |
| S2 | `./build-macos-full/tests/HydroCoupleSDKTests --gtest_filter='Differentiation*'` | **29 passed** |
| S3 | `BUILD=build-macos-full verification/g2/falsify.sh` | **32/32**; the Torch rows print SKIPPED |
| S4 | `cmake --build build-macos-torch --parallel && ./build-macos-torch/tests/HydroCoupleSDKTorchTests` | **20 passed** |
| S5 | `otool -L build-macos-torch/libHydroCoupleSDK*.dylib \| grep -i torch` | no output (the core SDK links no LibTorch) |
| S6 | `BUILD=build-macos-torch verification/g2/falsify.sh` | **46/46 caught**, none only because the build failed |
| C1 | `grep -rn "HydroCouple::SDK::Device\|MeshDataObjectType\|patchDimension\|IValueSemantics\|ITemporalSemantics" ../HydroCoupleComposer/src ../HydroCoupleComposer/include 2>/dev/null` | **Report only; do not port the Composer.** Its `tests/gui` stubs are known not to build against ABI 4. |

## 3. Pitfalls, and what you may fix

**Known pitfalls. Check these before you debug anything.**

- **P1. Wrong Python.** The `(base)` conda environment has no jax, so its
  two JAX checks skip. The falsifier now says `SKIPPED GATE` rather than
  `SURVIVED`.
  - Fix: activate `hydrocouple` and pass `PYTHON=python`.
  - Warning sign: H2 shows **179 passed, 10 skipped**.
- **P2. The SDK compiling against stale ABI 3 headers.** The SDK's
  CMakeLists tries `find_package(HydroCouple CONFIG)` first, and only then
  falls back to `../HydroCouple/include`. An installed interface package
  (a vcpkg port, or `HydroCouple/install/Darwin`) would give it the ABI 3
  headers.
  - Check `grep HYDROCOUPLE_INCLUDE_DIR build-macos-full/CMakeCache.txt build-macos-torch/CMakeCache.txt`.
    It must point at `…/HydroCouple/include`.
  - If it doesn't, reconfigure with `-DHYDROCOUPLE_INCLUDE_DIR=$PWD/../HydroCouple/include`,
    or reinstall the interface first.
  - Symptoms: `IValueDefinition::type()` "marked override but does not
    override", or `HYDROCOUPLE_ABI_VERSION` 3 in a log.
- **P3. Configure-time version lock.** The SDK enforces that its
  MAJOR.MINOR matches the interface headers' `version.h`. If the round
  bumped one side and not the other, configure fails with a clear message.
  - Bump the lagging side in the direction the CHANGELOGs state (SDK 2.1.0).
  - Report it.
- **P4. Falsifier race.** Only on Unix Makefiles; already fixed in b84bca7.
  - A falsifier row that reads `CAUGHT (does not build)` or `SURVIVED`
    must be reproduced by hand before you believe it: apply the mutation
    by hand, build, and run the gate.
  - Restore with `git diff` afterwards.
- **P5. `DeviceBackend` ambiguity.** Any other file that has both
  `using namespace HydroCouple;` and
  `using namespace HydroCouple::SDK::Device;` fails to compile.
  - Fix it by qualifying the name, as in `test_device.cpp`.
- **P6. Interrupted falsifier.** After Ctrl-C, both scripts restore their
  mutant. After a `kill -9` they cannot: run `git status` and restore the
  mutated file before trusting the tree.

**You may fix without asking:**

- Environment and harness problems (P1–P4, P6).
- Compile errors from mechanical ABI 4 fallout (renames, qualification,
  `override` changes).
- A test that fails for a reason you can explain in one sentence, where the
  fix does not weaken what the test proves.

Each fix is a minimal diff. Every fix goes in the §5 report with its file,
cause and effect.

**Stop and report, do not fix:**

- A numerical failure in any `Differentiation*` or `TorchDifferentiation*`
  test.
- A falsifier `SURVIVED` that you can reproduce by hand.
- Any failure whose fix would need an interface change.
- Anything that would mean deleting, skipping or loosening a test, or
  editing a falsifier row so it passes.

## 4. When everything is green

1. **Write verification records**, in the same shape as the existing
   `MAC_VERIFICATION_*` files. Keep the logs of the H3, S3 and S6 runs,
   including any failed first run, as evidence.
   - `HydroCouple/verification/g0g1/MAC_VERIFICATION_ABI4_2026-09-29.md`
     plus the H3 log.
   - `HydroCoupleSDK/verification/g2/MAC_VERIFICATION_ABI4_2026-09-29.md`
     plus the S3 and S6 logs.
2. **Commit HydroCouple first**, then the SDK. Do not push. Do not add an
   attribution line (Caleb's rule).
   - HydroCouple: the 26 round files, `verification/g0g1/falsify.sh`,
     `python/environment.yml`, and the record and log. **Not** `plans/`,
     **not** `install/`.
     Message: `feat(interface): contract-consistency round (ABI 4)`, with
     a body that points at the CHANGELOG and summarizes the check results.
   - SDK: the 53 files and the record and logs.
     Message: `feat: port to interface ABI 4`, with a body listing the four
     fixes and the check results.
   - Commit any fix of your own in the same commit, and describe it in the
     body.
3. **Don't touch `plans/` except this:** append a "Mac result" section to
   `plans/sdk/ABI4_HANDOFF_2026-09-29.md` (checks, fixes, commit hashes).

If anything is not green, commit nothing. Report instead.

## 5. The report you end with

End with this block, filled in. It is what gets pasted back.

```
ABI 4 on the Mac — result: <ALL GREEN, committed | NOT GREEN, nothing committed>

Environment: hydrocouple — Python <v>, numpy <v>, Cython <v>, torch <v>, jax <v>
  (<any change to environment.yml, and why>)
Interface include dir used by the SDK builds: <path>

Check  Expected                 This Mac
H1     153                      <…>
H2     186 passed, 3 skipped    <…>
H3     24/24                    <…>
S1     358 + 1 skip             <…>
S2     29                       <…>
S3     32/32 (T rows SKIPPED)   <…>
S4     20                       <…>
S5     no torch in core         <…>
S6     46/46                    <…>
C1     Composer references      <files and counts, or none>

Fixes I made (file — cause — effect):
  <none | one line each>

Failures I did not fix (and why):
  <none | test, message, what I found>

Commits (not pushed):
  HydroCouple: <hash> <subject>
  HydroCoupleSDK: <hash> <subject>

Anything the planning agent should know before G2.4:
  <…>
```

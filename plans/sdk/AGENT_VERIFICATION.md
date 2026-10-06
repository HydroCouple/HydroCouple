# HydroCoupleSDK — Agent Verification & Build Instructions

Instructions for an agent (or developer) to verify, build, and correct the
HydroCoupleSDK v2 work. The active checkout is
`~/Documents/Projects/cbuahin_github/HydroCoupleSDK`, branch `removing_qt`
(the copy under `Projects/HydroCouple/HydroCoupleSDK` is deprecated).

## Ground rules

1. **No Qt.** The SDK must not depend on Qt in any form. If a change
   introduces `QObject`, `QString`, `Q_OBJECT`, or `#include <Q...>`
   anywhere, it is wrong.
2. **vcpkg + CMake** manage dependencies and the build, following
   openswmm.engine conventions: `vcpkg.json` manifest with feature-gated
   optional deps (`tests`, `gdal`, `netcdf`, `hdf5`, `mpi`, `tools`),
   `CMakePresets.json` wiring `$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake`.
   CMake `USE_<X>` options default OFF to match the manifest's empty
   `default-features`.
3. **Interface contract.** The SDK implements the HydroCouple
   v2.0.0-alpha.1 headers (sibling checkout `../HydroCouple/include`,
   branch `dev`, tag `v2.0.0-alpha.1`). There is no `hydrocouple_variant`
   anywhere — data moves through
   `shape()/dataKind()/getValuesInto()/setValuesFrom()` only.
4. **Namespace and layout.** Implementation classes live in
   `HydroCouple::SDK` (`SDK::Temporal`, `SDK::Spatial`, later `SDK::IO`,
   `SDK::Tools`); the bare `HydroCouple` namespace belongs to the interface.
   Headers live under `include/hydrocouplesdk/{core,component,data,temporal,spatial}/`,
   mirrored by `src/hydrocouplesdk/`; the export macro header is
   `hydrocouplesdk/export.h`; the PCH is `src/stdafx.h` (never installed).
   Never place a header named after a libc header (e.g. `signal.h`) directly
   under `include/` — it shadows the system header.
5. **Tests + docs with every change.** New classes ship with GTest suites in
   `tests/hydrocouplesdk/` in the same change, and with full Doxygen
   documentation in the interface repo's style (`\file`, `\author Caleb
   Buahin`, `\version`, `\brief`, `\param`/`\return`, `\threadsafety`).
6. **Authorship.** Author is **Caleb Buahin** (never "Caleb Amoa Buahin");
   no email addresses may appear in any file.
7. **Plans.** Working plan markdowns live in `plans/` and are NOT committed
   (gitignored), except the historical tracked `plans/IMPLEMENTATION_PLAN.md`.

## Build and test (developer machine)

```bash
export VCPKG_ROOT=<path-to-vcpkg>
cmake --preset macos          # or linux / windows
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Pass criterion: configure, build, and all tests green (55 tests at the time
of writing; suites: BufferIO, Argument1D/2D, IdBasedArgument, ExchangeItem,
ModelComponent, TimeData, TimeSpanTest, TimeSeriesTest, TimeSeriesItemTest,
TimeSeriesArgumentTest).

Manual configure without presets:

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_MANIFEST_FEATURES=tests -DBUILD_TESTS=ON \
  -DHYDROCOUPLE_INCLUDE_DIR=<path>/HydroCouple/include
```

## Build and test (Claude sandbox — no vcpkg, no network CDNs)

The sandbox has no vcpkg and blocks most CDN downloads. Use the pre-staged
fallbacks:

- CMake: `python3 -c "import cmake, os; print(os.path.join(cmake.CMAKE_BIN_DIR,'cmake'))"`
  (pip cmake 4.x; bare `cmake` is not on PATH).
- GTest: prebuilt at `/tmp/gtest-install` (pass `-DCMAKE_PREFIX_PATH=/tmp/gtest-install`).
  If missing, build gtest from a git clone (github.com is reachable; release
  asset CDNs are not).
- nlohmann/json: single header at `/tmp/nl/nlohmann/json.hpp`
  (pass `-DNLOHMANN_JSON_INCLUDE_DIR=/tmp/nl`). If missing, sparse-clone
  nlohmann/json tag v3.11.3 and copy `single_include/nlohmann/json.hpp`.
- Interface headers: mount `/sessions/<name>/mnt/HydroCouple/include`.

```bash
CMAKE=$(python3 -c "import cmake, os; print(os.path.join(cmake.CMAKE_BIN_DIR,'cmake'))")
$CMAKE -S . -B /tmp/sdkbuild -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release \
  -DUSE_GDAL=OFF -DUSE_NETCDF=OFF -DUSE_HDF5=OFF -DUSE_OPENMP=OFF -DUSE_MPI=OFF \
  -DBUILD_TOOLS=OFF -DBUILD_TESTS=ON -DBUILD_DOCS=OFF \
  -DHYDROCOUPLE_INCLUDE_DIR=<interface>/include \
  -DNLOHMANN_JSON_INCLUDE_DIR=/tmp/nl -DCMAKE_PREFIX_PATH=/tmp/gtest-install
$CMAKE --build /tmp/sdkbuild --parallel
/tmp/sdkbuild/tests/HydroCoupleSDKTests
```

Fast per-file check while editing (no link):

```bash
g++ -std=c++20 -fsyntax-only -I include -I src -I <interface>/include -I /tmp/nl \
    src/hydrocouplesdk/<group>/<file>.cpp
```

## Correcting errors — recurring patterns

- **Undefined references to `Argument1D<T>::...` etc. at link time**: the
  library builds with `-fvisibility=hidden`; every explicit template
  instantiation (and its `extern template` declaration) must carry
  `HYDROCOUPLESDK_EXPORT`, e.g.
  `template class HYDROCOUPLESDK_EXPORT Argument1D<int>;`.
- **"template with C linkage" explosions from system headers**: a header
  named like a libc header is shadowing it from an include root (ground
  rule 4).
- **`'X' is not a namespace-name` / unresolved interface types in
  SDK::Temporal / SDK::Spatial files**: the file must include the interface
  header (`hydrocoupletemporal.h` / `hydrocouplespatial.h`) before its
  namespace block; each such file imports the interface names with
  `using namespace HydroCouple::Temporal;` (resp. `Spatial`) just inside
  the SDK sub-namespace.
- **`type 'HydroCouple::SDK::X' is not a direct base` after moving code**:
  an explicit `HydroCouple::X` qualification points at the interface
  namespace; SDK classes must be qualified `HydroCouple::SDK::X`.
- **`error: expected primary-expression before '>'` in templates**: dependent
  name — write `val.template get<T>()`.
- **`Severity`/`code` type errors in tests**: `Severity` is nested
  (`ErrorEntry::Severity`); `ErrorEntry::code` is `int32_t`, not a string.
- **GCC 11 has no `<format>`** — don't include it.
- **raster.cpp fails with `gdal_priv.h: No such file`** in the sandbox:
  expected; that unit is gated behind `-DUSE_GDAL=ON` and needs the `gdal`
  vcpkg feature. Do not "fix" it by stubbing GDAL.

## Notes

- Tests write file artifacts to `tests/artifacts/` (gitignored) so a human
  can review them — never to `/tmp`.
- File deletion works on this checkout's mount (enable via
  `allow_cowork_file_delete` if an `rm` is refused).

## Verification checklist

1. `grep -rn "hydrocouple_variant" include src tests` → no matches.
2. `grep -rln "Q_OBJECT\|QString\|#include <Q" include src tests` → no matches.
3. `grep -rn "Caleb Amoa\|@gmail.com" --exclude-dir=.git --exclude-dir=plans .`
   → no matches (this file's rule statement is the only tolerated mention).
4. Full build + ctest green with `-DVCPKG_MANIFEST_FEATURES=tests` (or the
   sandbox fallbacks above).
5. `cmake --build build --target docs` succeeds with Doxygen ≥ 1.9 and
   `-DBUILD_DOCS=ON` (interface-repo theme assets in `docs/custom/`), with
   no new warnings.
6. Versions agree: CMake `project(... VERSION 2.0.0)` +
   `HYDROCOUPLESDK_VERSION_SUFFIX "-alpha.1"`, `vcpkg.json`
   `"version-semver": "2.0.0-alpha.1"`, Doxygen `PROJECT_NUMBER`.
   The SDK versions in lockstep with the interface definitions: configure
   enforces MAJOR.MINOR equality against
   `${HYDROCOUPLE_INCLUDE_DIR}/version.h` (fatal on mismatch, status line
   "versions aligned" on success). When the interface bumps, bump the SDK
   in the same change.

## MPI in the sandbox

No system MPI and no root: install real MPICH through the mpi4py project's
binary wheels — `pip3 install --user mpich` puts `mpicc`/`mpirun` and
`mpi.h` under `~/.local`. Configure with `-DUSE_MPI=ON
-DCMAKE_PREFIX_PATH=$HOME/.local` and run the MPI suite with
`mpirun -n 3 tests/HydroCoupleSDKMpiTests` (tests skip gracefully below
their rank requirement, so `-n 1` is safe in constrained CI).

## Kokkos in the sandbox

Build from GitHub (release CDNs are blocked): clone `kokkos/kokkos` tag
4.5.01, configure with `-DKokkos_ENABLE_SERIAL=ON
-DCMAKE_POSITION_INDEPENDENT_CODE=ON` (PIC is mandatory — the static
Kokkos archives must link into the shared SDK), install to `~/.local`,
then configure the SDK with `-DUSE_KOKKOS=ON`. The device conformance
suite runs the same tests over host and kokkos backends.

## CDT in the sandbox

Header-only, MIT: clone `artem-ogre/CDT` tag 1.4.1 and configure the SDK
with `-DBUILD_TOOLS=ON -DCDT_INCLUDE_DIR=<clone>/CDT/include`. Via vcpkg
the `tools` feature provides the CDT::CDT config target instead.

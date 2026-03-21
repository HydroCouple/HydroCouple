# HydroCouple

**HydroCouple** is a header-only C++20 library that defines the core interface specifications for component-based, integrated water-resources modeling. It provides pure-virtual (`I`-prefixed) abstract interfaces for model components, exchange items, spatial/temporal data, and geospatial geometry — analogous to and extending the Open Modeling Interface (OpenMI) standard.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://github.com/HydroCouple/HydroCouple/blob/master/License) | [![Build and Package](https://github.com/HydroCouple/HydroCouple/actions/workflows/build_and_package.yml/badge.svg)](https://github.com/HydroCouple/HydroCouple/actions/workflows/build_and_package.yml) | [![Build and Unit Test](https://github.com/HydroCouple/HydroCouple/actions/workflows/build_and_unit_test.yml/badge.svg)](https://github.com/HydroCouple/HydroCouple/actions/workflows/build_and_unit_test.yml) | [![Build Docs](https://github.com/HydroCouple/HydroCouple/actions/workflows/build_docs.yml/badge.svg)](https://github.com/HydroCouple/HydroCouple/actions/workflows/build_docs.yml)

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Requirements](#requirements)
- [Project Structure](#project-structure)
- [Building](#building)
  - [CMake Presets](#cmake-presets)
  - [Manual CMake Configure](#manual-cmake-configure)
  - [CMake Options](#cmake-options)
  - [Installing](#installing)
  - [Packaging with CPack](#packaging-with-cpack)
- [Consuming with CMake](#consuming-with-cmake)
- [Consuming with vcpkg](#consuming-with-vcpkg)
- [API Overview](#api-overview)
  - [hydrocouple.h — Core Interfaces](#hydrocouplerh--core-interfaces)
  - [hydrocoupletemporal.h — Temporal Interfaces](#hydrocoupletemporalh--temporal-interfaces)
  - [hydrocouplespatial.h — Geospatial Interfaces](#hydrocouplespatialh--geospatial-interfaces)
  - [hydrocouplespatialwkb.h — WKB Geometry Structures](#hydrocouplespatialwkbh--wkb-geometry-structures)
  - [hydrocouplespatiotemporal.h — Spatio-Temporal Interfaces](#hydrocouplespatiotemporalh--spatio-temporal-interfaces)
  - [version.h — Build Metadata](#versionh--build-metadata)
- [Testing](#testing)
- [Continuous Integration](#continuous-integration)
- [Documentation](#documentation)
- [Citation](#citation)
- [License](#license)
- [References](#references)

---

## Overview

HydroCouple version **2.0.0** defines five header files — each targeting a distinct modeling concern — that together describe a full component coupling framework:

| Header | Namespace | Concern |
|---|---|---|
| `hydrocouple.h` | `HydroCouple` | Core model component lifecycle, exchange items, signals/slots, value types |
| `hydrocoupletemporal.h` | `HydroCouple::Temporal` | Time representation, time-marching components, time-series data items |
| `hydrocouplespatial.h` | `HydroCouple::Spatial` | OGC Simple Features geometry, mesh, network, raster, regular grid |
| `hydrocouplespatialwkb.h` | *(global)* | C-compatible OGC WKB binary geometry structures |
| `hydrocouplespatiotemporal.h` | `HydroCouple::SpatioTemporal` | Combined spatio-temporal component data items |

All interfaces use pure-virtual destructors and are designed to be implemented by concrete model components, with a name convention of `I` prefix (e.g., `IModelComponent`, `IOutput`).

---

## Features

- **Header-only** — zero compiled artifacts; link with `HydroCouple::HydroCouple` as a CMake INTERFACE target.
- **C++20** — uses `std::variant`, `std::span`, `std::any`, `std::unique_ptr`, and `[[nodiscard]]` throughout.
- **ABI-versioned** — `HydroCouple::HYDROCOUPLE_ABI_VERSION = 2`.
- **Signal/Slot** — lightweight, type-safe observer pattern (`ISignal<Args...>` / `ISlot<Args...>`) built into the interface hierarchy, replacing Qt/OpenMI dependencies.
- **OGC compliant** — geometry hierarchy follows the OGC Simple Features Access (SFA) specification; WKB structures match ISO 19125 binary encoding.
- **Cross-platform** — tested on Linux (x64), macOS (Apple Silicon), and Windows (x64) via GitHub Actions.
- **CMake package** — installs `HydroCoupleConfig.cmake` and `HydroCoupleConfigVersion.cmake` with `SameMajorVersion` compatibility.
- **vcpkg port** — first-class vcpkg support; installable as `hydrocouple`.
- **NuGet packaging** — CPack generates `.nupkg` files per platform via GitHub Packages.

---

## Requirements

| Tool | Minimum Version |
|---|---|
| CMake | 3.19 |
| C++ Compiler | C++20-capable (GCC 11+, Clang 13+, MSVC 19.29+) |
| vcpkg *(optional)* | any recent version |
| GoogleTest *(tests only)* | any version compatible with vcpkg `gtest` port |

No runtime library dependencies are required — all headers use only the C++ standard library.

---

## Project Structure

```
HydroCouple/
├── CMakeLists.txt              # Build configuration (v2.0.0, header-only INTERFACE target)
├── CMakePresets.json           # Presets for Windows, Linux, macOS
├── vcpkg.json                  # vcpkg manifest (hydrocouple 2.0.0)
├── vcpkg-configuration.json    # vcpkg registry configuration
├── License                     # MIT License
├── CITATION.cff                # Citation metadata
├── cmake/
│   └── HydroCoupleConfig.cmake.in  # CMake package config template
├── include/
│   ├── hydrocouple.h           # Core interfaces (HydroCouple namespace)
│   ├── hydrocoupletemporal.h   # Temporal interfaces (HydroCouple::Temporal)
│   ├── hydrocouplespatial.h    # Geospatial interfaces (HydroCouple::Spatial)
│   ├── hydrocouplespatialwkb.h # WKB geometry structures (global)
│   ├── hydrocouplespatiotemporal.h # Spatio-temporal interfaces (HydroCouple::SpatioTemporal)
│   └── version.h.in            # Version header template (configure_file input)
├── tests/
│   ├── CMakeLists.txt          # Test executable: hydrocouple_tests (GTest)
│   ├── test_hydrocouple.cpp    # Core type and interface property tests
│   └── test_spatial_wkb.cpp    # WKB struct layout and enum value tests
└── .github/workflows/
    ├── build_and_unit_test.yml # CI: build + ctest on Linux/macOS/Windows
    ├── build_and_package.yml   # CI: CPack + NuGet publish
    └── build_docs.yml          # CI: Doxygen + GitHub Pages deploy
```

---

## Building

### CMake Presets

Three platform presets are provided in `CMakePresets.json`. All inherit from the `default` (Ninja) preset which sets `CMAKE_TOOLCHAIN_FILE` to `$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake`.

| Configure Preset | Generator | Condition |
|---|---|---|
| `default` | Ninja | any host |
| `windows` | Visual Studio 17 2022 (x64) | `hostSystemName == Windows` |
| `linux` | Unix Makefiles | `hostSystemName == Linux` |
| `macos` | Xcode | `hostSystemName == Darwin` |

```bash
# Configure using a preset (requires VCPKG_ROOT set)
cmake --preset linux

# Build
cmake --build --preset linux
```

### Manual CMake Configure

```bash
cmake -B build \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build
```

### CMake Options

| Option | Default | Description |
|---|---|---|
| `HYDROCOUPLE_BUILD_TESTS` | `OFF` | Build the GoogleTest unit test suite. Requires the `gtest` vcpkg port. |

```bash
cmake -B build -DHYDROCOUPLE_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

### Installing

```bash
cmake --install build --prefix /usr/local
```

This installs:

- **Headers** (`hydrocouple.h`, `hydrocouplespatial.h`, `hydrocouplespatialwkb.h`, `hydrocouplespatiotemporal.h`, `hydrocoupletemporal.h`, generated `version.h`) → `${prefix}/include/`
- **CMake config files** (`HydroCoupleConfig.cmake`, `HydroCoupleConfigVersion.cmake`, `HydroCoupleTargets.cmake`) → `${prefix}/lib/cmake/HydroCouple/`
- **License** → `${prefix}/share/HydroCouple/`

### Packaging with CPack

```bash
cmake --build build --target package
```

Generates:
- **Windows:** `HydroCouple-2.0.0-win64.zip` + `HydroCouple.x64.windows.2.0.0.nupkg`
- **Linux:** `HydroCouple-2.0.0-linux.tar.gz` + `HydroCouple.x64.linux.2.0.0.nupkg`
- **macOS:** `HydroCouple-2.0.0-macos.tar.gz` + `HydroCouple.arm64.osx.2.0.0.nupkg`

NuGet packages are published to the [HydroCouple GitHub Packages feed](https://nuget.pkg.github.com/HydroCouple/index.json).

---

## Consuming with CMake

After installation or using a vcpkg-integrated build:

```cmake
find_package(HydroCouple REQUIRED)

target_link_libraries(my_model PRIVATE HydroCouple::HydroCouple)
```

The `HydroCouple::HydroCouple` target is an `INTERFACE` library. Linking it propagates:
- Include directory for the HydroCouple headers
- The `HYDROCOUPLE_LIBRARY` compile definition
- The `cxx_std_20` compile feature requirement

---

## Consuming with vcpkg

Add HydroCouple as a dependency in your `vcpkg.json`:

```json
{
  "dependencies": [
    "hydrocouple"
  ]
}
```

To include unit tests:

```json
{
  "dependencies": [
    { "name": "hydrocouple", "features": ["tests"] }
  ]
}
```

The vcpkg manifest (`vcpkg.json`) declares the following:

| Field | Value |
|---|---|
| `name` | `hydrocouple` |
| `version-semver` | `2.0.0` |
| `license` | `MIT` |
| `homepage` | `https://hydrocouple.org` |
| Host dependencies | `vcpkg-cmake`, `vcpkg-cmake-config` |
| Optional feature `tests` | adds `gtest` |
| Default features | `tests` |

The registry is configured to the official Microsoft vcpkg registry at baseline `13bde2ff13192e1b2fdd37bd9b475c7665ae6ae5`.

---

## API Overview

### hydrocouple.h — Core Interfaces

**Namespace:** `HydroCouple`

Defines the fundamental abstractions for the component coupling framework.

**Global type:**
- `ByteOrder` — `enum class ByteOrder : uint8_t { BigEndian = 0, LittleEndian = 1 }`

**Constants:**
- `HYDROCOUPLE_ABI_VERSION = 2`

**Value type:**
- `hydrocouple_variant` — `std::variant<std::monostate, bool, char, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, float, double, long double, std::string, std::any>` — the unified value type used across all exchange items and component data items.
- `hydrocouple_variant_less` — comparator enabling use of `hydrocouple_variant` in ordered containers.
- `hydrocouple_variant_set` — `std::set<hydrocouple_variant, hydrocouple_variant_less>`.

**Signal/Slot infrastructure:**

| Interface | Description |
|---|---|
| `ISlot<Args...>` | Listener interface; implement `operator()(const ISignal<Args...>&, Args...)` |
| `ISignal<Args...>` | Observable interface; `connect()`, `disconnect()`, `blockSignals()`, `emit()` |
| `IPropertyChanged` | Extends `ISignal<std::string>`; emits the changed property name |

**Identity and description:**

| Interface | Inherits | Description |
|---|---|---|
| `IDescription` | `IPropertyChanged` | `caption()`, `setCaption()`, `description()`, `setDescription()` |
| `IIdentity` | `IDescription` | Adds `id()` (unique within context) |

**Component metadata:**

| Interface | Inherits | Description |
|---|---|---|
| `IComponentInfo` | `IIdentity` | Metadata factory: `libraryFilePath`, `iconFilePath`, `developer`, `documentation`, `license`, `copyright`, `url`, `email`, `version`, `tags`, `validateLicense()` |
| `IModelComponentInfo` | `IComponentInfo` | `createComponentInstance()` → `unique_ptr<IModelComponent>`; `adaptedOutputFactories()` |
| `IAdaptedOutputFactoryComponentInfo` | `IComponentInfo` | Metadata for adapted output factory components |
| `IWorkflowComponentInfo` | `IComponentInfo` | Metadata for workflow components |

**Model component:**

| Interface | Description |
|---|---|
| `IModelComponent` | Core component interface. Extends `IIdentity` + `ISignal<IComponentStatusChangeEventArgs>`. Methods: `componentInfo()`, `initialize()`, `validate()`, `prepare()`, `update()`, `finish()`, `inputs()`, `outputs()`, `arguments()`, `status()` |
| `IModelComponent::ComponentStatus` | Lifecycle enum: `Created`, `Initializing`, `Initialized`, `Validating`, `Valid`, `WaitingForData`, `Invalid`, `Preparing`, `Updated`, `Done`, `Failed` |
| `IProxyModelComponent` | Extends `IModelComponent`; remote/proxy execution |
| `IComponentStatusChangeEventArgs` | Carries old/new `ComponentStatus`; attached message |
| `ICloneableModelComponent` | Extends `IModelComponent`; adds `clone()` |

**Value definitions and units:**

| Interface | Description |
|---|---|
| `IValueDefinition` | Extends `IDescription`; describes the type of a data item's values |
| `IDimension` | Extends `IIdentity`; describes a single axis/dimension |
| `IQuality` | Extends `IValueDefinition`; categorical data (ordinal/nominal) |
| `IUnitDimensions` | Extends `IDescription`; fundamental SI dimension exponents |
| `IUnit` | Extends `IDescription`; conversion factor/offset; `DistanceUnits` enum; links to `IUnitDimensions` |
| `IQuantity` | Extends `IValueDefinition`; continuous numerical data; links to `IUnit` |

**Component data items:**

| Interface | Description |
|---|---|
| `IComponentDataItem` | Extends `IIdentity` + `ISignal<IComponentDataItemValueChanged>`. Has dimensions, value definition, and get/set value methods |
| `IComponentDataItemValueChanged` | Event args for data value change notifications |
| `IArgument` | Extends `IComponentDataItem`; read/write parameter passed to a component at configuration time |
| `IIdBasedComponentDataItem` | Extends `IComponentDataItem`; data indexed by string IDs |

**Exchange items:**

| Interface | Inherits | Description |
|---|---|---|
| `IExchangeItem` | `IComponentDataItem` | Base for inputs and outputs |
| `IOutput` | `IExchangeItem` | Provides data; holds list of `IInput` consumers and `IAdaptedOutput` decorators. `getValue()` triggers computation |
| `IAdaptedOutput` | `IOutput` | Decorates an `IOutput`; transforms data to be compatible with a connected `IInput` |
| `IAdaptedOutputFactory` | `IIdentity` | Creates `IAdaptedOutput` instances for mismatched output-input pairs |
| `IAdaptedOutputFactoryComponent` | `IAdaptedOutputFactory` | Loadable factory component |
| `IInput` | `IExchangeItem` | Consumes data from a connected `IOutput` |
| `IMultiInput` | `IInput` | Consumes data from multiple `IOutput` sources |
| `IExchangeItemChangeEventArgs` | — | Event args for exchange item change notifications |

**Workflow:**

| Interface | Description |
|---|---|
| `IWorkflowComponent` | Orchestrates a set of `IModelComponent` instances; extends `IIdentity` + `ISignal<IWorkflowComponentStatusChangeEventArgs>` |
| `IWorkflowComponentStatusChangeEventArgs` | Status change event for workflow components |

---

### hydrocoupletemporal.h — Temporal Interfaces

**Namespace:** `HydroCouple::Temporal`

| Interface | Inherits | Description |
|---|---|---|
| `IDateTime` | `IPropertyChanged` | Julian day (`julianDay()`), modified Julian day (`modifiedJulianDay()`), serial date (`serialDate()`) |
| `ITimeSpan` | `IDateTime` | A time interval; adds `duration()` in days |
| `ITimeModelComponent` | `IModelComponent` | Time-advancing component; `currentDateTime()`, `simulationPeriod()` → `ITimeSpan*` |
| `ITimeSeriesComponentDataItem` | `IComponentDataItem` | Multi-timestep data item with `IDateTime` time dimension; get/set value at a `timeIndex` |
| `ITimeIdBasedComponentDataItem` | `IComponentDataItem` | Time-varying data indexed by a string ID dimension |

---

### hydrocouplespatial.h — Geospatial Interfaces

**Namespace:** `HydroCouple::Spatial`

**Enumerations:**

| Enum | Values |
|---|---|
| `MeshDataObjectType` | `Cell`, `Vertex`, `Edge`, `Face` |
| `NetworkDataObjectType` | `Node`, `Edge` |
| `SpatialDataType` | `Scalar`, `MultiScalar`, `Vector`, `Tensor` |
| `RegularGridType` | `Cartesian`, `Rectilinear`, `Curvilinear` |
| `IGeometry::GeometryType` | Full OGC SFA set: `Point=1`, `LineString=2`, `Polygon=3`, `Triangle=17`, `MultiPoint=4`, `MultiLineString=5`, `MultiPolygon=6`, `GeometryCollection=7`, `CircularString=8`, `CompoundCurve=9`, `CurvePolygon=10`, `PolyhedralSurface=15`, `TIN=16`; plus `*Z` (+1000), `*M` (+2000), `*ZM` (+3000) variants |

**Spatial reference and envelope:**

| Interface | Description |
|---|---|
| `ISpatialReferenceSystem` | SRID (`authSRID()`), authority name (`authName()`), WKT (`srText()`), distance units |
| `IEnvelope` | Bounding box: `minX/maxX/minY/maxY/minZ/maxZ` |

**OGC Simple Features geometry hierarchy:**

| Interface | Inherits | Description |
|---|---|---|
| `IGeometry` | — | Root geometry; `id()`, `index()`, `geometryType()`, `spatialReferenceSystem()`, `envelope()`, `isEmpty()`, `isSimple()`, `is3D()`, `isMeasured()`, WKB export, and full OGC topological predicates/operators |
| `IGeometryCollection` | `IGeometry` | Multi-geometry; `numGeometries()`, `geometry(int)` |
| `IPoint` | `IGeometry` | `x()`, `y()`, `z()`, `m()` |
| `IMultiPoint` | `IGeometryCollection` | Collection of points |
| `IVertex` | `IPoint` | Topologically aware point; `edge()` access |
| `ICurve` | `IGeometry` | `length()`, `startPoint()`, `endPoint()`, `isClosed()`, `isRing()` |
| `IMultiCurve` | `IGeometryCollection` | `length()`, `isClosed()` |
| `ILineString` | `ICurve` | `numPoints()`, `point(int)` |
| `IMultiLineString` | `IMultiCurve` | Collection of line strings |
| `ILine` | `ILineString` | Exactly 2 points |
| `ILinearRing` | `ILineString` | Closed ring |
| `IEdge` | — | Topological half-edge; `next()`, `prev()`, `rot()`, `invRot()`, `sym()`, `orig()`, `dest()`, `left()`, `right()` |
| `ISurface` | `IGeometry` | `area()`, `centroid()`, `pointOnSurface()`, `boundingPolygon()` |
| `IMultiSurface` | `IGeometryCollection` | `area()`, `centroid()`, `pointOnSurface()` |
| `IPolygon` | `ISurface` | `exteriorRing()`, `numInteriorRings()`, `interiorRing(int)` |
| `IMultiPolygon` | `IMultiSurface` | Collection of polygons |
| `ITriangle` | `IPolygon` | Triangle (3-vertex polygon) |
| `INetwork` | `IIdentity` | Directed/undirected network graph: `nodes()`, `edges()`, `spatialReferenceSystem()` |
| `IPolyhedralSurface` | `ISurface` | Contiguous set of polygonal patches: `numPatches()`, `patch(int)`, `boundingPolygons(IPolygon*)` |
| `ITIN` | `IPolyhedralSurface` | Triangulated Irregular Network |

**Raster and grid:**

| Interface | Description |
|---|---|
| `IRaster` | Grid raster: `numColumns()`, `numRows()`, `numBands()`, `xCellSize()`, `yCellSize()`, `xllCorner()`, `yllCorner()`, `noDataValue()`, `spatialReferenceSystem()`, `band(int)` |
| `IRasterBand` | Single band of a raster: `raster()`, `data(col, row)`, `setData(col, row, value)` |
| `IRegularGrid2D` | 2D structured grid: `numXNodes()`, `numYNodes()`, `x(xIndex, yIndex)`, `y(xIndex, yIndex)`, cell/face/node counts |
| `IRegularGrid3D` | 3D structured grid: extends 2D with `numZNodes()`, `z(...)` |

**Component data items (spatial):**

| Interface | Inherits | Description |
|---|---|---|
| `IGeometryComponentDataItem` | `IComponentDataItem` | Values indexed over an `IGeometryCollection`; get/set by `geometryIndex` + optional extra dimension indexes |
| `INetworkComponentDataItem` | `IComponentDataItem` | Values on network nodes or edges |
| `IPolyhedralSurfaceComponentDataItem` | `IComponentDataItem` | Values on polyhedral surface patches, vertices, or edges |
| `ITINComponentDataItem` | `IComponentDataItem` | Values on TIN triangles/vertices/edges |
| `IRasterComponentDataItem` | `IComponentDataItem` | Values on raster cells indexed by column and row (and band) |
| `IRegularGrid2DComponentDataItem` | `IComponentDataItem` | Values on 2D grid cells, faces, or nodes |
| `IRegularGrid3DComponentDataItem` | `IComponentDataItem` | Values on 3D grid cells, faces, or nodes |

---

### hydrocouplespatialwkb.h — WKB Geometry Structures

**Namespace:** global (C-compatible)

Provides C-compatible `struct`/`union` types for parsing and constructing OGC ISO 19125 Well-Known Binary (WKB) geometry representations. All structs are `trivially_copyable` and `standard_layout` for safe `memcpy` and serialization.

**Enumerations:**

| Enum | Underlying type | Values |
|---|---|---|
| `WKBByteOrder` | `uint8_t` | `wkbXDR=0` (big-endian), `wkbNDR=1` (little-endian) |
| `WKBGeometryType` | `uint32_t` | `wkbPoint=1`, `wkbLineString=2`, `wkbPolygon=3`, `wkbMultiPoint=4`, `wkbMultiLineString=5`, `wkbMultiPolygon=6`, `wkbGeometryCollection=7`, `wkbPolyhedralSurface=15`, `wkbTIN=16`, `wkbTriangle=17`; plus `*Z` (+1000), `*M` (+2000), `*ZM` (+3000) variants |

**Coordinate structs:**

`Point`, `PointZ`, `PointM`, `PointZM` — double-precision coordinates.  
`LinearRing`, `LinearRingZ`, `LinearRingM`, `LinearRingZM` — counted arrays of coordinate structs.

**WKB geometry structs:**

Each geometry type has four variants (2D, Z, M, ZM):  
`WKBPoint[Z|M|ZM]`, `WKBLineString[Z|M|ZM]`, `WKBPolygon[Z|M|ZM]`,  
`WKBTriangle[Z|M|ZM]`, `WKBPolyhedralSurface[Z|M|ZM]`, `WKBTIN[Z|M|ZM]`,  
`WKBMultiPoint[Z|M|ZM]`, and additional collection types.

**Discriminated unions:**

`WKBGeometry`, `WKBGeometryZ`, `WKBGeometryM`, `WKBGeometryZM` — unions over all same-dimensionality WKB types, enabling generic geometry handling.

---

### hydrocouplespatiotemporal.h — Spatio-Temporal Interfaces

**Namespace:** `HydroCouple::SpatioTemporal`

Combines `HydroCouple::Temporal::ITimeSeriesComponentDataItem` with the spatial component data item interfaces to form time-varying spatial data items.

| Interface | Inherits | Description |
|---|---|---|
| `ITimeGeometryComponentDataItem` | `ITimeSeriesComponentDataItem` + `IGeometryComponentDataItem` | Values indexed by `(timeIndex, geometryIndex)` over a geometry collection |
| `ITimeNetworkComponentDataItem` | `ITimeSeriesComponentDataItem` + `INetworkComponentDataItem` | Time-varying network node/edge values |
| `ITimeSeriesPolyhedralSurfaceComponentDataItem` | `ITimeSeriesComponentDataItem` + `IPolyhedralSurfaceComponentDataItem` | Time-varying polyhedral surface values |
| `ITimeSeriesTINComponentDataItem` | `ITimeSeriesPolyhedralSurfaceComponentDataItem` | Time-varying TIN values |
| `ITimeSeriesRasterComponentDataItem` | `ITimeSeriesComponentDataItem` + `IRasterComponentDataItem` | Time-varying raster cell values |
| `ITimeRegularGrid2DComponentDataItem` | `ITimeSeriesComponentDataItem` + `IRegularGrid2DComponentDataItem` | Time-varying 2D grid values |
| `ITimeRegularGrid3DComponentDataItem` | `ITimeSeriesComponentDataItem` + `IRegularGrid3DComponentDataItem` | Time-varying 3D grid values |

---

### version.h — Build Metadata

Generated at configure time from `include/version.h.in` into `${CMAKE_CURRENT_BINARY_DIR}/include/version.h`. Provides the following preprocessor macros:

| Macro | Example value |
|---|---|
| `PROJECT_NAME` | `"HydroCouple"` |
| `PROJECT_VERSION_MAJOR` | `2` |
| `PROJECT_VERSION_MINOR` | `0` |
| `PROJECT_VERSION_PATCH` | `0` |
| `PROJECT_VERSION` | `"2.0.0"` |
| `BUILD_ID` | UUID derived from `https://www.hydrocouple.org` |
| `BUILD_COMMIT` | Git SHA of `HEAD` |
| `BUILD_BRANCH` | Git branch name |
| `BUILD_DATE` | UTC timestamp at configure time |

---

## Testing

Unit tests are built when `HYDROCOUPLE_BUILD_TESTS=ON`. The test executable is `hydrocouple_tests`, linked against the `HydroCouple`, `GTest::gtest`, and `GTest::gtest_main` targets, and uses `gtest_discover_tests()` for CTest integration.

**Test file `test_hydrocouple.cpp`** covers:
- `ByteOrder` enum class values and implicit-conversion behaviour
- `hydrocouple_variant`: all signed/unsigned numeric types, `std::string`, `std::any`
- Interface type properties (abstract base class checks)

**Test file `test_spatial_wkb.cpp`** covers:
- `WKBByteOrder` enum class values, underlying type (`uint8_t`)
- `WKBGeometryType` enum class: 2D, Z, M, and ZM type codes
- `Point`, `PointZ`, `PointM`, `PointZM` struct sizes, `trivially_copyable`, `standard_layout`, `trivially_default_constructible`
- WKB geometry struct layouts

```bash
# Build and run tests
cmake -B build -DHYDROCOUPLE_BUILD_TESTS=ON \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## Continuous Integration

Three GitHub Actions workflows run on every push and pull request to `master` and `dev`:

| Workflow file | Name | What it does | Platforms |
|---|---|---|---|
| `build_and_unit_test.yml` | **Build and Unit Test** | Configures with `HYDROCOUPLE_BUILD_TESTS=ON`, builds, and runs `ctest` | ubuntu-latest (x64-linux), macos-latest (arm64-osx), windows-latest (x64-windows) |
| `build_and_package.yml` | **Build and Package** | Configures with `HYDROCOUPLE_BUILD_TESTS=OFF`, runs CPack to produce `.tar.gz`/`.zip`/`.nupkg`, pushes NuGet packages to GitHub Packages | ubuntu-latest, macos-latest, windows-latest |
| `build_docs.yml` | **Build Documentation** | Installs Doxygen + Graphviz, builds HTML docs, deploys to GitHub Pages, uploads `hydrocouple_docs.tar.gz` artifact | ubuntu-latest |

All workflows use vcpkg at ref `2025.02.14` and cache packages via the GitHub NuGet feed (`https://nuget.pkg.github.com/HydroCouple/index.json`).

---

## Documentation

Full Doxygen API documentation is available at:
[https://www.hydrocouple.org/hydrocoupledocs/html/index.html](https://www.hydrocouple.org/hydrocoupledocs/html/index.html)

---

## Citation

If you use HydroCouple in your research, please cite:

```
Buahin, C.A., Horsburgh, J.S., 2018. Advancing the Open Modeling Interface (OpenMI)
for integrated water resources modeling. Environmental Modelling & Software 108, 133–153.
https://doi.org/10.1016/j.envsoft.2018.07.015
```

**BibTeX:**
```bibtex
@article{buahin2018openmi,
  author  = {Buahin, Caleb A. and Horsburgh, Jeffery S.},
  title   = {Advancing the Open Modeling Interface ({OpenMI}) for integrated water resources modeling},
  journal = {Environmental Modelling \& Software},
  year    = {2018},
  volume  = {108},
  pages   = {133--153},
  doi     = {10.1016/j.envsoft.2018.07.015}
}
```

ORCID: [0000-0002-9859-2264](https://orcid.org/0000-0002-9859-2264)

---

## License

HydroCouple is released under the [MIT License](License).  
Copyright © 2025 HydroCouple / Caleb Buahin.

---

## References

### Foundational Publications

* Buahin, C.A., Horsburgh, J.S., 2018. Advancing the Open Modeling Interface (OpenMI) for integrated water resources modeling. *Environmental Modelling & Software* 108, 133–153. https://doi.org/10.1016/j.envsoft.2018.07.015

* Buahin, C. and J. Horsburgh, 2016. From OpenMI to HydroCouple: Advancing OpenMI to Support Experimental Simulations and Standard Geospatial Datasets. *International Congress on Environmental Modelling and Software*. Toulouse, France. http://scholarsarchive.byu.edu/iemssconference/2016/Stream-A/11

### Applications

* Buahin, C.A., Horsburgh, J.S., Neilson, B.T., 2019. Parallel multi-objective calibration of a component-based river temperature model. *Environmental Modelling & Software* 116, 57–71. https://doi.org/10.1016/j.envsoft.2019.02.012

* Mihalevich, B.A., Neilson, B.T., Buahin, C.A., Yackulic, C.B., Schmidt, J.C., 2020. Water Temperature Controls for Regulated Canyon-Bound Rivers. *Water Resources Research* 56, e2020WR027566. https://doi.org/10.1029/2020WR027566

* Mihalevich, B.A., Neilson, B.T., Buahin, C.A., 2022. Evaluation of the ERA5-Land Reanalysis Data Set for Process-Based River Temperature Modeling Over Data Sparse and Topographically Complex Regions. *Water Resources Research* 58, e2021WR031294. https://doi.org/10.1029/2021WR031294

* Leach, J.A., Neilson, B.T., Buahin, C.A., Moore, R.D., Laudon, H., 2021. Lake Outflow and Hillslope Lateral Inflows Dictate Thermal Regimes of Forested Streams Draining Small Lakes. *Water Resources Research* 57, e2020WR028136. https://doi.org/10.1029/2020WR028136

* Leach, J.A., Moore, R.D., Laudon, H., Buahin, C.A., Neilson, B.T., 2022. Do headwater lakes moderate downstream temperature response to forest harvesting? Illustrating opportunities and obstacles associated with virtual experiments. *Hydrological Processes* 36, e14593. https://doi.org/10.1002/hyp.14593


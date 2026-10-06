# HydroCoupleSDK — Implementation Plan

**Author:** Caleb Buahin  
**Date:** 2026-05-07  
**Version:** 1.0.0

---

## 1. Overview

This document specifies the complete modernisation of **HydroCoupleSDK** from a
Qt-dependent library into a portable, standards-conforming C++17 shared library.
Key goals:

| Goal | Approach |
|---|---|
| **Re-license to MIT** | Replace LGPL-3.0 with MIT in every file, CMake, and vcpkg manifest |
| Remove all Qt dependencies | Replace with C++20 STL + nlohmann/json |
| Modern build system | CMake 3.18 + vcpkg |
| Portable SIMD vectorisation | `#pragma omp simd`, SoA layouts, no platform intrinsics |
| Thread-safe file I/O | `std::shared_mutex`-protected NetCDF & HDF5 wrappers |
| Complete interface coverage | Every `HydroCouple::I*` interface has a concrete SDK class |
| Coherent folder + namespace structure | SDK mirrors the interface namespace hierarchy |
| Geospatial I/O | GDAL for all vector, raster, and argument file formats |
| Efficient unstructured mesh | Index-based Guibas-Stolfi quad-edge with SoA vertex pools |
| Full test coverage | Google Test for every class and every interface method |
| Doxygen documentation | All symbols documented; HTML output matching HydroCouple style |

---

## 2. Namespace & Folder Structure

The HydroCouple interface headers declare four C++ namespaces.
Every SDK implementation class lives in **the same namespace as the interface it implements**,
using the same folder hierarchy. The `I` prefix is dropped from the class name.

```
HydroCouple interfaces            SDK implementations
────────────────────────────────  ──────────────────────────────────────────────
HydroCouple::IDescription       → HydroCouple::Description
HydroCouple::IModelComponent    → HydroCouple::ModelComponent
HydroCouple::Temporal::IDateTime → HydroCouple::Temporal::DateTime
HydroCouple::Spatial::IPoint    → HydroCouple::Spatial::Point
HydroCouple::SpatioTemporal::
  ITimeGeometryComponentDataItem → HydroCouple::SpatioTemporal::TimeGeometryComponentDataItem
```

### 2.1 Header Layout

```
include/
├── HydroCoupleSDK.h               ← umbrella header (includes every module)
├── hydrocouplesdk.h               ← HYDROCOUPLESDK_EXPORT macro
├── hydrocoupleforward.h           ← all SDK class forward declarations
├── signal.h                       ← Signal<Args...> / FunctionSlot<Args...>
├── uuid.h                         ← generateUUID() — C++17 <random> only
├── version.h                      ← generated from version.h.in
│
├── hydrocouple/                   ← namespace HydroCouple
│   ├── hydrocouple.h              ← module aggregate
│   ├── description.h              ← Description     : IDescription
│   ├── identity.h                 ← Identity        : IIdentity
│   ├── componentinfo.h            ← ComponentInfo   : IComponentInfo
│   ├── modelcomponentinfo.h       ← ModelComponentInfo : IModelComponentInfo
│   ├── modelcomponent.h           ← ModelComponent  : IModelComponent
│   ├── cloneablemodelcomponent.h  ← CloneableModelComponent : ICloneableModelComponent
│   ├── workflowcomponent.h        ← WorkflowComponent : IWorkflowComponent
│   ├── workflowcomponentinfo.h    ← WorkflowComponentInfo : IWorkflowComponentInfo
│   ├── valuedefinition.h          ← ValueDefinition / Quality / Quantity
│   ├── unit.h                     ← Unit            : IUnit
│   ├── unitdimensions.h           ← UnitDimensions  : IUnitDimensions
│   ├── dimension.h                ← Dimension       : IDimension
│   ├── componentdataitem.h        ← ComponentDataItem1D<T> / ComponentDataItem2D<T>
│   ├── idbasedcomponentdataitem.h ← IdBasedComponentDataItem<T>
│   ├── argument.h                 ← Argument1D<T> / IdBasedArgument<T>
│   ├── exchangeitem.h             ← AbstractExchangeItem
│   ├── input.h                    ← Input / MultiInput
│   ├── output.h                   ← Output
│   ├── adaptedoutput.h            ← AdaptedOutput
│   ├── adaptedoutputfactory.h     ← AdaptedOutputFactory / AdaptedOutputFactoryComponent
│   ├── datacursor.h               ← DataCursor (indexing utility)
│   └── eventargs/
│       ├── componentstatuschangeeventargs.h
│       ├── componentdataitemvaluechanged.h
│       ├── exchangeitemchangeeventargs.h
│       └── workflowstatuschangeeventargs.h
│
├── temporal/                      ← namespace HydroCouple::Temporal
│   ├── temporal.h                 ← module aggregate
│   ├── datetime.h                 ← DateTime  : IDateTime
│   ├── timespan.h                 ← TimeSpan  : ITimeSpan
│   ├── timeseries.h               ← TimeSeries (CSV/file loader, not an interface)
│   ├── timemodelcomponent.h       ← TimeModelComponent : ITimeModelComponent
│   ├── timeseriescomponentdataitem.h
│   ├── timeidbasedcomponentdataitem.h
│   ├── timeseriesargument.h
│   ├── timeseriesinputs.h         ← TimeSeriesInput / TimeSeriesMultiInput
│   ├── timeseriesoutputs.h        ← TimeSeriesOutput
│   ├── timeidbasedinputs.h
│   ├── timeidbasedoutputs.h
│   └── interpolation/
│       ├── timeseriesinterpolationadaptedoutput.h
│       └── temporalinterpolationfactory.h
│
├── spatial/                       ← namespace HydroCouple::Spatial
│   ├── spatial.h                  ← module aggregate
│   │
│   ├── geometry/                  ← geometry types
│   │   ├── geometry.h             ← Geometry (abstract base) : IGeometry
│   │   ├── point.h                ← Point : IPoint
│   │   ├── vertex.h               ← Vertex : IVertex
│   │   ├── linestring.h           ← LineString / Line / LinearRing
│   │   ├── polygon.h              ← Polygon : IPolygon
│   │   ├── triangle.h             ← Triangle : ITriangle
│   │   ├── geometrycollection.h   ← GeometryCollection + Multi* variants
│   │   ├── envelope.h             ← Envelope : IEnvelope
│   │   └── spatialreferencesystem.h ← SpatialReferenceSystem : ISpatialReferenceSystem
│   │
│   ├── mesh/                      ← unstructured mesh (Guibas-Stolfi)
│   │   ├── edge.h                 ← Edge : IEdge  (lightweight handle)
│   │   ├── polyhedralsurface.h    ← PolyhedralSurface : IPolyhedralSurface
│   │   ├── tin.h                  ← TIN : ITIN
│   │   └── pools/
│   │       ├── quadedgepool.h     ← QuadEdgePool (index-based topology)
│   │       ├── vertexpool.h       ← VertexPool (SoA coordinates)
│   │       └── facepool.h         ← FacePool
│   │
│   ├── network/                   ← directed graph
│   │   └── network.h              ← Network : INetwork
│   │
│   ├── grid/                      ← regular grids
│   │   ├── regulargrid2d.h        ← RegularGrid2D : IRegularGrid2D
│   │   └── regulargrid3d.h        ← RegularGrid3D : IRegularGrid3D
│   │
│   ├── raster/                    ← GDAL-backed raster
│   │   └── raster.h               ← Raster / RasterBand
│   │
│   ├── index/                     ← spatial indices
│   │   └── octree.h               ← Octree
│   │
│   ├── componentdataitems/
│   │   ├── geometrycomponentdataitem.h
│   │   ├── networkcomponentdataitem.h
│   │   ├── polyhedralsurfacecomponentdataitem.h
│   │   ├── tincomponentdataitem.h
│   │   ├── rastercomponentdataitem.h
│   │   ├── regulargrid2dcomponentdataitem.h
│   │   └── regulargrid3dcomponentdataitem.h
│   │
│   ├── arguments/
│   │   ├── geometryargument.h
│   │   └── tinargument.h
│   │
│   ├── exchangeitems/
│   │   ├── geometryexchangeitems.h
│   │   └── tinexchangeitems.h
│   │
│   └── io/
│       └── geometryfactory.h      ← GDAL import/export utilities
│
├── spatiotemporal/                ← namespace HydroCouple::SpatioTemporal
│   ├── spatiotemporal.h           ← module aggregate
│   ├── timegeometrycomponentdataitem.h
│   ├── timenetworkcomponentdataitem.h
│   ├── timeseriespolyhedralsurfacecomponentdataitem.h
│   ├── timeseriestincomponentdataitem.h
│   ├── timeseriesrastercomponentdataitem.h
│   ├── timeregulargrid2dcomponentdataitem.h
│   ├── timeregulargrid3dcomponentdataitem.h
│   └── exchangeitems/
│       ├── timegeometryexchangeitems.h
│       └── timetinexchangeitems.h
│
└── io/                            ← namespace HydroCouple::IO
    ├── netcdf/
    │   ├── threadsafenetcdf.h     ← module aggregate
    │   ├── threadsafencfile.h
    │   ├── threadsafencgroup.h
    │   ├── threadsafencvar.h
    │   ├── threadsafencdim.h
    │   ├── threadsafencatt.h
    │   └── nchydrocouple.h        ← HydroCouple-level NetCDF utilities
    └── hdf5/
        ├── threadsafehdf5.h       ← module aggregate
        ├── threadsafeh5file.h
        ├── threadsafeh5group.h
        ├── threadsafeh5dataset.h
        ├── threadsafeh5attribute.h
        └── h5hydrocouple.h        ← HydroCouple-level HDF5 utilities
```

### 2.2 Source Layout

```
src/
├── hydrocouple/
├── temporal/
├── spatial/
│   ├── geometry/
│   ├── mesh/
│   │   └── pools/
│   ├── network/
│   ├── grid/
│   ├── raster/
│   ├── index/
│   ├── componentdataitems/
│   ├── arguments/
│   ├── exchangeitems/
│   └── io/
├── spatiotemporal/
├── io/
│   ├── netcdf/
│   └── hdf5/
└── vendor/
    └── tinyxml2/          ← vendored, compiled as part of the library
```

### 2.3 Namespace → Folder Mapping

| Namespace | Include folder | Source folder |
|---|---|---|
| `HydroCouple` | `include/hydrocouple/` | `src/hydrocouple/` |
| `HydroCouple::Temporal` | `include/temporal/` | `src/temporal/` |
| `HydroCouple::Spatial` | `include/spatial/` | `src/spatial/` |
| `HydroCouple::SpatioTemporal` | `include/spatiotemporal/` | `src/spatiotemporal/` |
| `HydroCouple::IO::NetCDF` | `include/io/netcdf/` | `src/io/netcdf/` |
| `HydroCouple::IO::HDF5` | `include/io/hdf5/` | `src/io/hdf5/` |

> **Migration note:** `include/core/` is **renamed** `include/hydrocouple/` and every
> class in it is wrapped in `namespace HydroCouple { ... }`. No other folder is renamed.

---

## 3. Build System

### 3.1 vcpkg.json

```json
{
  "name": "hydrocouplesdk",
  "version-semver": "1.0.0",
  "description": "HydroCouple C++ Modeling Framework SDK",
  "license": "LGPL-3.0",
  "homepage": "https://www.hydrocouple.org",
  "dependencies": [
    { "name": "vcpkg-cmake",        "host": true },
    { "name": "vcpkg-cmake-config", "host": true },
    "gdal",
    "netcdf-cxx4",
    "hdf5",
    "tinyxml2",
    { "name": "gtest", "default-features": false },
    { "name": "openmp", "platform": "!uwp" }
  ],
  "features": {
    "mpi": {
      "description": "Enable MPI support",
      "dependencies": ["mpi"]
    }
  }
}
```

### 3.2 vcpkg-configuration.json

```json
{
  "default-registry": {
    "kind": "git",
    "baseline": "13bde2ff13192e1b2fdd37bd9b475c7665ae6ae5",
    "repository": "https://github.com/microsoft/vcpkg"
  },
  "registries": [
    {
      "kind": "artifact",
      "location": "https://github.com/microsoft/vcpkg-ce-catalog/archive/refs/heads/main.zip",
      "name": "microsoft"
    }
  ]
}
```

### 3.3 CMakePresets.json

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "default",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build",
      "cacheVariables": {
        "CMAKE_TOOLCHAIN_FILE": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      }
    },
    { "name": "windows", "inherits": "default", "generator": "Visual Studio 17 2022",
      "architecture": { "value": "x64" },
      "condition": { "type": "equals", "lhs": "${hostSystemName}", "rhs": "Windows" } },
    { "name": "linux",   "inherits": "default", "generator": "Unix Makefiles",
      "condition": { "type": "equals", "lhs": "${hostSystemName}", "rhs": "Linux" } },
    { "name": "macos",   "inherits": "default", "generator": "Xcode",
      "condition": { "type": "equals", "lhs": "${hostSystemName}", "rhs": "Darwin" } }
  ],
  "buildPresets": [
    { "name": "default", "hidden": true, "configurePreset": "default" },
    { "name": "windows", "configurePreset": "windows" },
    { "name": "linux",   "configurePreset": "linux"   },
    { "name": "macos",   "configurePreset": "macos"   }
  ]
}
```

### 3.4 CMakeLists.txt (top-level)

Key points matching the HydroCouple interface project style:

- `cmake_minimum_required(VERSION 3.18)`
- `set(CMAKE_CXX_STANDARD 17)`, `set(CMAKE_CXX_EXTENSIONS OFF)`
- `include(CheckCXXCompilerFlag)` — probe for `-fvisibility=hidden` portably
- `find_package` via vcpkg for GDAL, netCDF, HDF5, tinyxml2, GTest, OpenMP, MPI
- `configure_file(include/version.h.in include/version.h)` — git metadata (same as HydroCouple)
- Feature options: `USE_NETCDF ON`, `USE_HDF5 ON`, `USE_OPENMP ON`, `USE_MPI OFF`, `BUILD_TESTS ON`, `BUILD_DOCS OFF`
- Install rules mirroring HydroCouple: `GNUInstallDirs`, `CMakePackageConfigHelpers`, CPack
- `add_subdirectory(src)`, `add_subdirectory(tests)`, `add_subdirectory(docs)`

### 3.5 include/version.h.in

```cpp
#ifndef HYDROCOUPLESDK_VERSION_H
#define HYDROCOUPLESDK_VERSION_H
#define HYDROCOUPLESDK_NAME          "@PROJECT_NAME@"
#define HYDROCOUPLESDK_VERSION_MAJOR  @PROJECT_VERSION_MAJOR@
#define HYDROCOUPLESDK_VERSION_MINOR  @PROJECT_VERSION_MINOR@
#define HYDROCOUPLESDK_VERSION_PATCH  @PROJECT_VERSION_PATCH@
#define HYDROCOUPLESDK_VERSION       "@PROJECT_VERSION@"
#define HYDROCOUPLESDK_BUILD_ID      "@BUILD_ID@"
#define HYDROCOUPLESDK_BUILD_COMMIT  "@BUILD_COMMIT@"
#define HYDROCOUPLESDK_BUILD_BRANCH  "@BUILD_BRANCH@"
#define HYDROCOUPLESDK_BUILD_DATE    "@BUILD_DATE@"
#endif
```

---

## 4. Foundation Headers

### 4.1 `include/hydrocouplesdk.h` — Export Macro

Portable chain: MSVC `__declspec`, GCC/Clang `__attribute__`, fallback no-op:

```cpp
#if defined(_WIN32) || defined(_WIN64)
  #ifdef HYDROCOUPLESDK_LIBRARY
    #define HYDROCOUPLESDK_EXPORT __declspec(dllexport)
  #else
    #define HYDROCOUPLESDK_EXPORT __declspec(dllimport)
  #endif
#elif defined(__GNUC__) || defined(__clang__)
  #define HYDROCOUPLESDK_EXPORT __attribute__((visibility("default")))
#else
  #define HYDROCOUPLESDK_EXPORT
#endif
```

### 4.2 `include/signal.h` — Concrete Signal/Slot

Implements `HydroCouple::ISignal<Args...>` and `HydroCouple::ISlot<Args...>`:

```cpp
namespace HydroCouple {

template<typename... Args>
class Signal : public virtual ISignal<Args...> {
    std::vector<std::weak_ptr<ISlot<Args...>>> m_slots;
    bool m_blocked = false;
    mutable std::shared_mutex m_mutex;
public:
    void connect(const std::shared_ptr<ISlot<Args...>> &slot) override;
    void disconnect(const std::shared_ptr<ISlot<Args...>> &slot) override;
    void blockSignals(bool block) override;
protected:
    void emit(Args... args) override;
    // emit() takes a snapshot copy under unique_lock, then fires outside the lock
    // to allow re-entrant connect/disconnect. Expired weak_ptrs are purged lazily.
};

// Convenience: wrap a std::function as an ISlot
template<typename... Args>
class FunctionSlot : public ISlot<Args...> {
    std::function<void(const ISignal<Args...>&, Args...)> m_fn;
public:
    explicit FunctionSlot(std::function<void(const ISignal<Args...>&, Args...)> fn)
        : m_fn(std::move(fn)) {}
    void operator()(const ISignal<Args...> &sender, Args... args) override {
        m_fn(sender, std::forward<Args>(args)...);
    }
};

} // namespace HydroCouple
```

### 4.3 `include/uuid.h` — UUID Utility

Pure C++17 `<random>` + `<sstream>` + `<iomanip>`. No platform UUID APIs:

```cpp
namespace HydroCouple {
    inline std::string generateUUID() {
        // Uses std::mt19937_64 seeded from std::random_device
        // Formats as RFC 4122 v4 UUID string
    }
} // namespace HydroCouple
```

### 4.4 `include/hydrocoupleforward.h` — Forward Declarations

All ~60 SDK concrete class names forward-declared in their correct namespaces.

---

## 5. Qt → C++17 Substitution

Work order (bottom-up through inheritance hierarchy):

1. `hydrocouple/description.h` — root class; adds `Signal<std::string> m_propertyChanged`
2. `hydrocouple/identity.h` — `std::string m_id`, UUID via `HydroCouple::generateUUID()`
3. `hydrocouple/valuedefinition.h` — `hydrocouple_variant` default/missing, `std::type_index`
4. `hydrocouple/unit.h`, `unitdimensions.h`, `dimension.h`
5. `hydrocouple/componentdataitem.h` — remove `QXmlStreamReader`; use `tinyxml2::XMLElement*`
6. `hydrocouple/argument.h` — XML/JSON/file round-trip via tinyxml2
7. `hydrocouple/modelcomponent.h` — replaces `QMutex → std::mutex`, `QDir → std::filesystem`, `QHash → std::unordered_map`, `QXmlStreamReader → tinyxml2`
8. All exchange items, adapted outputs, factories
9. `temporal/datetime.h` — remove `QDateTime`; keep `double m_julianDay` only; date parse via `std::get_time`
10. `temporal/timeseries.h` — `std::regex` delimiters; `std::ifstream`
11. `spatial/geometry/` — `std::string m_id`; `uint32_t` geometry flags; keep GDAL headers

### Substitution Table

| Qt | C++17 replacement |
|---|---|
| `Q_OBJECT`, `Q_INTERFACES`, `Q_PROPERTY`, `Q_DECLARE_METATYPE` | Remove entirely |
| `signals:` / `slots:` / `emit` | `Signal<Args...>` member + `.emit(...)` |
| `QObject` base class | Remove; classes implement HydroCouple interfaces directly |
| `QString` | `std::string` |
| `QVector<T>`, `QList<T>`, `QStringList` | `std::vector<T>` |
| `QHash<K,V>` | `std::unordered_map<K,V>` |
| `QSet<T>` | `std::unordered_set<T>` |
| `QSharedPointer<T>` | `std::shared_ptr<T>` |
| `QMutex` | `std::mutex` |
| `QThread` | `std::thread` |
| `QFile / QDir / QFileInfo / QDirIterator` | `std::filesystem` |
| `QXmlStreamReader / QXmlStreamWriter` | tinyxml2 (`XMLElement*` DOM API) |
| `QUuid::createUuid()` | `HydroCouple::generateUUID()` |
| `QDateTime` | Julian Day `double`; date parse via `std::get_time` |
| `QRegularExpression` | `std::regex` |
| `QDataStream / QTextStream` | `std::fstream`, `std::ostringstream` |
| `QVariant` | `HydroCouple::hydrocouple_variant` (`std::variant`) |
| `QCoreApplication` | Remove |
| `Q_DECLARE_FLAGS(Flags, Flag)` | `using Flags = uint32_t` + bitwise operators |

---

## 6. Spatial Data Structures — Detailed Design

### 6.1 Design Principles

The unstructured mesh uses the **Guibas-Stolfi quad-edge data structure**, which is
already employed. The refactored design replaces pointer-based AoS with **index-based SoA**:

| Current (AoS) | Refactored (SoA + index) |
|---|---|
| `Edge *m_next` in each Edge object | `std::vector<int32_t> m_next` in QuadEdgePool |
| `HCVertex *m_vertex` in Edge | `std::vector<int32_t> m_vertex` in QuadEdgePool |
| `double m_x, m_y, m_z` per HCVertex | `std::vector<double> x, y, z` in VertexPool |
| `new QuadEdge()` per edge pair | `QuadEdgePool::allocateQuad()` returns base index |
| `Edge *` pointer arithmetic for rot/sym | Integer arithmetic on pool indices |

**Key invariant preserved:** Within a QuadEdge block at base index `4k`,
edges occupy indices `4k`, `4k+1`, `4k+2`, `4k+3`. The combinatorial operations are:

```
rot(e)    = (e & ~3) | ((e + 1) & 3)   // identical semantics to current index < 3 ? this+1 : this-3
sym(e)    = e ^ 2                        // flip bit 1
invRot(e) = (e & ~3) | ((e + 3) & 3)
```

All topology traversal reduces to **integer arithmetic + single array lookup** — no pointer chasing.

### 6.2 `QuadEdgePool` (`include/spatial/mesh/pools/quadedgepool.h`)

```cpp
namespace HydroCouple::Spatial {

class HYDROCOUPLESDK_EXPORT QuadEdgePool {
public:
    explicit QuadEdgePool(int expectedQuads = 4096);

    // Returns base index (multiple of 4); initialises default next[] links
    int32_t allocateQuad();
    void    freeQuad(int32_t base);    // returns to free list; does not zero memory

    // ── Combinatorial ops (all constexpr inline) ─────────────────────────
    constexpr int32_t rot(int32_t e)    const noexcept { return (e&~3)|((e+1)&3); }
    constexpr int32_t invRot(int32_t e) const noexcept { return (e&~3)|((e+3)&3); }
    constexpr int32_t sym(int32_t e)    const noexcept { return e^2; }
    inline int32_t origNext(int32_t e)  const noexcept { return m_next[e]; }
    inline int32_t origPrev(int32_t e)  const noexcept { return rot(origNext(rot(e))); }
    inline int32_t destNext(int32_t e)  const noexcept { return sym(origNext(sym(e))); }
    inline int32_t destPrev(int32_t e)  const noexcept { return invRot(origNext(invRot(e))); }
    inline int32_t leftNext(int32_t e)  const noexcept { return rot(origNext(invRot(e))); }
    inline int32_t leftPrev(int32_t e)  const noexcept { return sym(origNext(e)); }
    inline int32_t rightNext(int32_t e) const noexcept { return invRot(origNext(rot(e))); }
    inline int32_t rightPrev(int32_t e) const noexcept { return origNext(sym(e)); }

    // Guibas-Stolfi Splice — 4 next-pointer swaps
    void splice(int32_t a, int32_t b) noexcept;

    // ── Data access ───────────────────────────────────────────────────────
    inline int32_t vertexOf(int32_t e)        const noexcept { return m_vertex[e]; }
    inline void    setVertexOf(int32_t e, int32_t v)  noexcept { m_vertex[e] = v; }
    inline int32_t faceOf(int32_t e)           const noexcept { return m_face[e]; }
    inline void    setFaceOf(int32_t e, int32_t f)    noexcept { m_face[e] = f; }
    inline int32_t marker(int32_t e)           const noexcept { return m_marker[e>>2]; }
    inline void    setMarker(int32_t e, int32_t m)    noexcept { m_marker[e>>2] = m; }

    int32_t edgeCapacity() const noexcept { return (int32_t)m_next.size(); }

    // ── SoA arrays (direct access for SIMD callers) ───────────────────────
    std::vector<int32_t> m_next;    // origNext index; size = 4 * numQuads
    std::vector<int32_t> m_vertex;  // vertex index for primal edges (e%4==0,2); -1 otherwise
    std::vector<int32_t> m_face;    // face index for dual edges (e%4==1,3); -1 otherwise
    std::vector<int32_t> m_marker;  // one per QuadEdge; size = numQuads

private:
    std::vector<int32_t> m_freeList;
};

} // namespace HydroCouple::Spatial
```

### 6.3 `VertexPool` (`include/spatial/mesh/pools/vertexpool.h`)

```cpp
namespace HydroCouple::Spatial {

class HYDROCOUPLESDK_EXPORT VertexPool {
public:
    explicit VertexPool(int expectedVertices = 4096);

    int32_t allocate(double x, double y, double z = 0.0, double m = 0.0);
    void    free(int32_t idx);

    // ── SoA coordinate arrays ─────────────────────────────────────────────
    std::vector<double>  x, y, z, m;    // coordinate channels; same length
    std::vector<int32_t> firstEdge;      // index of one outgoing primal edge per vertex
    std::vector<uint8_t> flags;          // bit0=hasZ, bit1=hasM, bit2=active

    inline bool hasZ(int32_t i)     const noexcept { return (flags[i] & 1) != 0; }
    inline bool hasM(int32_t i)     const noexcept { return (flags[i] & 2) != 0; }
    inline bool isActive(int32_t i) const noexcept { return (flags[i] & 4) != 0; }

    int32_t size() const noexcept { return (int32_t)x.size(); }

    // ── SIMD-vectorised bulk helpers ──────────────────────────────────────
    // Copy XYZ of `n` vertices (given by index array) into caller buffers.
    void bulkGetXYZ(const int32_t *indices, int n,
                    double *out_x, double *out_y, double *out_z) const noexcept;

    // Compute squared distances from query point (qx, qy, qz) to n vertices.
    void bulkDistSq(const int32_t *indices, int n,
                    double qx, double qy, double qz,
                    double *out_distSq) const noexcept;

private:
    std::vector<int32_t> m_freeList;
};

} // namespace HydroCouple::Spatial
```

`bulkGetXYZ` implementation:
```cpp
void VertexPool::bulkGetXYZ(const int32_t *idx, int n,
                             double *ox, double *oy, double *oz) const noexcept {
#ifdef _OPENMP
#pragma omp simd
#endif
    for (int i = 0; i < n; ++i) { ox[i] = x[idx[i]]; oy[i] = y[idx[i]]; oz[i] = z[idx[i]]; }
}
```

### 6.4 `FacePool` (`include/spatial/mesh/pools/facepool.h`)

```cpp
namespace HydroCouple::Spatial {

class HYDROCOUPLESDK_EXPORT FacePool {
public:
    explicit FacePool(int expectedFaces = 2048);
    int32_t allocate();
    void    free(int32_t idx);

    std::vector<int32_t> firstEdge;  // one dual edge index per face
    std::vector<uint8_t> flags;      // bit0 = active

    int32_t size() const noexcept { return (int32_t)firstEdge.size(); }

private:
    std::vector<int32_t> m_freeList;
};

} // namespace HydroCouple::Spatial
```

### 6.5 `Edge` — Lightweight Handle (`include/spatial/mesh/edge.h`)

```cpp
namespace HydroCouple::Spatial {

class HYDROCOUPLESDK_EXPORT Edge final : public virtual HydroCouple::Spatial::IEdge {
    friend class PolyhedralSurface;
    friend class TIN;
    friend class Network;
public:
    Edge() = default;
    Edge(int32_t index, QuadEdgePool *ep, VertexPool *vp, FacePool *fp)
        : m_index(index), m_edgePool(ep), m_vertexPool(vp), m_facePool(fp) {}

    unsigned int index() const override { return static_cast<unsigned int>(m_index); }

    // IEdge topology (virtual dispatch, each delegates to pool inline)
    IEdge   *rot()      override;
    IEdge   *invRot()   override;
    IEdge   *sym()      override;
    IEdge   *origNext() override;
    IEdge   *origPrev() override;
    IEdge   *destNext() override;
    IEdge   *destPrev() override;
    IEdge   *leftNext() override;
    IEdge   *leftPrev() override;
    IEdge   *rightNext() override;
    IEdge   *rightPrev() override;

    IVertex  *orig()  override;
    IVertex  *dest()  override;
    IPolygon *left()  override;
    IPolygon *right() override;
    IPolygon *face()  override;

    // Internal (typed, non-virtual) — zero overhead
    Edge    *rotInternal()    const noexcept;
    Edge    *symInternal()    const noexcept;
    Vertex  *origInternal()   const noexcept;
    Vertex  *destInternal()   const noexcept;
    Polygon *leftInternal()   const noexcept;
    Polygon *rightInternal()  const noexcept;

    double   length()         const noexcept;
    int32_t  rawIndex()       const noexcept { return m_index; }

    static void splice(Edge *a, Edge *b) noexcept;  // delegates to pool

private:
    int32_t       m_index     = -1;
    QuadEdgePool *m_edgePool  = nullptr;
    VertexPool   *m_vertexPool = nullptr;
    FacePool     *m_facePool   = nullptr;
};

} // namespace HydroCouple::Spatial
```

### 6.6 `Vertex` Handle (`include/spatial/geometry/vertex.h`)

```cpp
namespace HydroCouple::Spatial {

class HYDROCOUPLESDK_EXPORT Vertex : public virtual HydroCouple::Spatial::IVertex {
    friend class PolyhedralSurface;
    friend class TIN;
    friend class Network;
public:
    Vertex() = default;
    Vertex(int32_t idx, VertexPool *vp, QuadEdgePool *ep)
        : m_index(idx), m_pool(vp), m_edgePool(ep) {}

    // IPoint
    double x() const override { return m_pool->x[m_index]; }
    double y() const override { return m_pool->y[m_index]; }
    double z() const override { return m_pool->z[m_index]; }
    double m() const override { return m_pool->m[m_index]; }
    void setX(double v) { m_pool->x[m_index] = v; }
    void setY(double v) { m_pool->y[m_index] = v; }
    void setZ(double v) { m_pool->z[m_index] = v; }

    // IVertex
    IEdge *edge() const override;  // uses m_pool->firstEdge[m_index]

    int32_t rawIndex() const noexcept { return m_index; }

private:
    int32_t      m_index    = -1;
    VertexPool  *m_pool     = nullptr;
    QuadEdgePool *m_edgePool = nullptr;
};

} // namespace HydroCouple::Spatial
```

### 6.7 `PolyhedralSurface` and `TIN`

**`include/spatial/mesh/polyhedralsurface.h`:**

```cpp
namespace HydroCouple::Spatial {

class HYDROCOUPLESDK_EXPORT PolyhedralSurface
    : public HydroCouple::Spatial::Geometry,
      public virtual HydroCouple::Spatial::IPolyhedralSurface {
public:
    explicit PolyhedralSurface(const std::string &id = HydroCouple::generateUUID());

    // Capacity reservation (must be called before construction to prevent
    // handle-vector reallocation, which would invalidate Edge*/Vertex* pointers)
    void reserveVertices(int n);
    void reserveEdges(int n);    // n = number of QuadEdges (= 4n edge slots)
    void reserveFaces(int n);

    // IPolyhedralSurface
    int         patchCount()  const override;
    IPolygon   *patch(int i)  const override;
    int         vertexCount() const override;
    IVertex    *vertex(int i) const override;
    IMultiPolygon *boundingPolygons(const IPolygon *) const override;
    bool        isClosed()    const override;
    void enable3D() override;  void disable3D() override;
    void enableM()  override;  void disableM()  override;

    // Topology construction (same semantics as current HCPolyhedralSurface)
    Edge *createVertexEdge(Vertex *orig, Vertex *dest, Polygon *left, Polygon *right);
    Edge *createFaceEdge(Polygon *patch, Vertex *orig, Vertex *dest);
    void  deleteVertexEdge(Edge *edge);
    void  deleteFaceEdge(Edge *edge);

    // Internal accessors (no virtual dispatch)
    Vertex  *vertexInternal(int i)  const noexcept;
    Polygon *patchInternal(int i)   const noexcept;
    Edge    *edgeHandleFor(int32_t idx) const noexcept;
    Vertex  *vertexHandleFor(int32_t idx) const noexcept;

protected:
    // ── Pools (own all memory) ────────────────────────────────────────────
    VertexPool    m_vertexPool;
    QuadEdgePool  m_edgePool;
    FacePool      m_facePool;

    // ── Stable handle arrays (reserved upfront; never reallocated) ────────
    std::vector<Edge>    m_edgeHandles;
    std::vector<Vertex>  m_vertexHandles;
    std::vector<Polygon> m_patchHandles;

    // ── Ordered index lists (insertion order; allow O(1) indexed access) ──
    std::vector<int32_t> m_patchIndices;
    std::vector<int32_t> m_vertexIndices;

    int64_t m_patchCntr = 0, m_vertexCntr = 0;

private:
    // Static orbit helpers — logically unchanged; use pool indices
    static Edge *getOrbitOrg(Edge *e, Vertex *org);
    static void  setOrbitOrg(Edge *e, Vertex *org);
    static Edge *getOrbitLeft(Edge *e, Polygon *left);
    static Edge *getOrbitRight(Edge *e, Polygon *right);
    static void  setOrbitLeft(Edge *e, Polygon *left);
    static Edge *getClosestOrbitLeftNull(Vertex *origin, Vertex *destination);
};

class HYDROCOUPLESDK_EXPORT TIN
    : public PolyhedralSurface,
      public virtual HydroCouple::Spatial::ITIN {
public:
    explicit TIN(const std::string &id = HydroCouple::generateUUID());

    ITriangle *triangle(int index) const override;
    Triangle  *triangleInternal(int index) const noexcept;

    // Create a triangle from three existing vertices
    Triangle *createTriangle(Vertex *v1, Vertex *v2, Vertex *v3);

    // Triangulate using the Triangle library (Shewchuk)
    static TIN *triangulateSFS(const std::string &command,
                               const std::vector<Point*> &points,
                               const std::vector<LineString*> &constraints,
                               const std::vector<Point*> &holes);

    // GDAL I/O
    static TIN *readFromLayer(OGRLayer *layer);
    bool writeToLayer(OGRLayer *layer, std::string &errorMessage) const;

    Edge *createFaceEdge(Polygon *patch, Vertex *orig, Vertex *dest) override;
};

} // namespace HydroCouple::Spatial
```

### 6.8 `Network` (`include/spatial/network/network.h`)

```cpp
namespace HydroCouple::Spatial {

class HYDROCOUPLESDK_EXPORT Network
    : public HydroCouple::Identity,
      public virtual HydroCouple::Spatial::INetwork {
public:
    explicit Network(const std::string &id = HydroCouple::generateUUID());

    void reserveVertices(int n);
    void reserveEdges(int n);

    int     edgeCount()   const override;
    IEdge  *edge(int i)   const override;
    int     vertexCount() const override;
    IVertex *vertex(int i) const override;

    // Create directed edge; inserts into the correct angular position
    // in the origNext orbit (atan2 angle ordering)
    Edge *createVertexEdge(Vertex *origin, Vertex *destination);
    void  deleteVertexEdge(Edge *edge);

private:
    void addVertex(Vertex *v);   bool removeVertex(Vertex *v);
    void addEdge(Edge *e);       bool removeEdge(Edge *e);
    Edge *findEdge(Vertex *origin, Vertex *destination) const;
    Edge *getClosestOrbitLeft(Vertex *origin, Vertex *destination);

    VertexPool   m_vertexPool;
    QuadEdgePool m_edgePool;

    std::vector<Edge>    m_edgeHandles;
    std::vector<Vertex>  m_vertexHandles;
    std::vector<int32_t> m_vertexIndices;
    std::vector<int32_t> m_edgeIndices;  // primal edge (idx % 4 == 0) indices only
};

} // namespace HydroCouple::Spatial
```

### 6.9 `LineString` — SoA Point Storage

```cpp
namespace HydroCouple::Spatial {

class HYDROCOUPLESDK_EXPORT LineString
    : public Geometry,
      public virtual HydroCouple::Spatial::ILineString {
public:
    // Adds a point by copying coordinates into SoA arrays
    void addPoint(double x, double y, double z = 0.0);
    void addPoint(const IPoint *p);
    int pointCount() const override;

    // Point accessor returns a lightweight on-stack value object (no heap)
    Point pointAt(int i) const;   // returns by value — no allocation

    // SIMD-vectorised length
    double length() const override;

    // Direct SoA access for geometry operations
    const double *xData() const noexcept { return m_x.data(); }
    const double *yData() const noexcept { return m_y.data(); }
    const double *zData() const noexcept { return m_z.data(); }

private:
    std::vector<double> m_x, m_y, m_z, m_m;
    bool m_hasZ = false, m_hasM = false;
};

} // namespace HydroCouple::Spatial
```

**Vectorised length:**
```cpp
double LineString::length() const {
    double total = 0.0;
    const int n = (int)m_x.size() - 1;
#ifdef _OPENMP
#pragma omp simd reduction(+:total)
#endif
    for (int i = 0; i < n; ++i) {
        double dx = m_x[i+1]-m_x[i], dy = m_y[i+1]-m_y[i], dz = m_z[i+1]-m_z[i];
        total += std::sqrt(dx*dx + dy*dy + dz*dz);
    }
    return total;
}
```

### 6.10 `Vect` — Stack-Allocated 3-Vector

Remove the heap-allocated `double *v = nullptr` member; replace with `double v[3]`:

```cpp
struct HYDROCOUPLESDK_EXPORT Vect {
    double v[3] = {0.0, 0.0, 0.0};  // 24 bytes on stack, always in L1
    // All existing methods unchanged — operator[], dot, cross, normalize, etc.
    // Remove constructor that calls new[], remove destructor delete[]
};
```

### 6.11 `RegularGrid2D` and `RegularGrid3D`

```cpp
namespace HydroCouple::Spatial {

class HYDROCOUPLESDK_EXPORT RegularGrid2D
    : public HydroCouple::Identity,
      public virtual HydroCouple::Spatial::IRegularGrid2D {
public:
    // Factory methods
    static RegularGrid2D *createCartesian(double x0, double y0,
                                          double dx, double dy, int nx, int ny);
    static RegularGrid2D *createCurvilinear(const std::vector<double> &nodeX,
                                            const std::vector<double> &nodeY,
                                            int nxNodes, int nyNodes);
    static RegularGrid2D *fromGDALDataset(GDALDataset *ds);

    // IRegularGrid2D
    ISpatialReferenceSystem *spatialReferenceSystem() const override;
    IRegularGrid2D::GridType gridType()               const override;
    int    numXNodes()                                const override;
    int    numYNodes()                                const override;
    double xNodeLocation(int xi, int yi)              const override;
    double yNodeLocation(int xi, int yi)              const override;
    bool   isActive(int xCell, int yCell)             const override;

    // Direct array access for SIMD callers
    const double *nodeXData() const noexcept { return m_nodeX.data(); }
    const double *nodeYData() const noexcept { return m_nodeY.data(); }
    int totalNodes()          const noexcept { return m_nx * m_ny; }

private:
    int m_nx = 0, m_ny = 0;
    std::vector<double>  m_nodeX, m_nodeY;   // row-major [y*nx + x]
    std::vector<uint8_t> m_active;            // one byte per cell
    IRegularGrid2D::GridType m_gridType = IRegularGrid2D::Rectilinear;
    std::shared_ptr<SpatialReferenceSystem> m_srs;
};

} // namespace HydroCouple::Spatial
```

---

## 7. Component Data Items — Flat SoA Value Arrays

All mesh/grid component data items store their scalar data in a **single flat**
`std::vector<T>` indexed by a compound key. The layout depends on `MeshDataType`:

| DataType | Index formula | Array size |
|---|---|---|
| `Cell` | `patchIndex` | `numPatches` |
| `Edge` | `edgeIndex` | `numEdges` |
| `Node` | `vertexIndex` | `numVertices` |
| `CellEdge` | `patchIndex * 3 + localEdge` | `numPatches * 3` |
| `CellNode` | `patchIndex * 3 + localNode` | `numPatches * 3` |

For time-varying items the layout is **time-outer, space-inner**:
```
index = timeIndex * spatialStride + spatialIndex
```
This makes a full spatial snapshot at time `t` a contiguous block of `spatialStride`
values — copyable with `std::copy` and fully auto-vectorisable by the compiler.

### 7.1 `TINComponentDataItem<T>` (`include/spatial/componentdataitems/tincomponentdataitem.h`)

```cpp
namespace HydroCouple::Spatial {

template<typename T>
class HYDROCOUPLESDK_EXPORT TINComponentDataItem
    : public HydroCouple::AbstractComponentDataItem,
      public virtual HydroCouple::Spatial::ITINComponentDataItem {
public:
    TINComponentDataItem(const std::string &id,
                         HydroCouple::Spatial::MeshDataType dataType,
                         std::shared_ptr<TIN> tin,
                         HydroCouple::Dimension *cellDim,
                         HydroCouple::Dimension *edgeDim,
                         HydroCouple::Dimension *nodeDim,
                         HydroCouple::ValueDefinition *valueDef,
                         HydroCouple::ModelComponent *component);

    // ITINComponentDataItem
    ITIN              *TIN()              const override;
    IPolyhedralSurface *polyhedralSurface() const override;
    IDimension        *cellDimension()    const override;
    IDimension        *edgeDimension()    const override;
    IDimension        *nodeDimension()    const override;
    MeshDataType       meshDataType()     const override;

    // IComponentDataItem
    void getValue(hydrocouple_variant &data,
                  const std::initializer_list<int> &dims) const override;
    void getValues(hydrocouple_variant *data,
                   const std::initializer_list<int> &dims,
                   const std::initializer_list<int> &lengths = {}) const override;
    void setValue(const hydrocouple_variant &data,
                  const std::initializer_list<int> &dims) override;
    void setValues(const hydrocouple_variant *data,
                   const std::initializer_list<int> &dims,
                   const std::initializer_list<int> &lengths = {}) override;

    // Fast bulk copy (SIMD-eligible, contiguous memory)
    void bulkGet(int offset, int count, T *out) const noexcept;
    void bulkSet(int offset, int count, const T *in)  noexcept;
    void resetDataArray();

    std::vector<T> m_data;  // flat, indexed per layout table above

private:
    std::shared_ptr<TIN>              m_tin;
    HydroCouple::Spatial::MeshDataType m_dataType;
    HydroCouple::Dimension *m_cellDim, *m_edgeDim, *m_nodeDim;
};

} // namespace HydroCouple::Spatial
```

### 7.2 `NetworkComponentDataItem<T>` — same pattern, edge/vertex indexed

### 7.3 `RegularGrid2DComponentDataItem<T>` — row-major flat array, hyperslab bulk copy

---

## 8. GDAL I/O — Complete Coverage

All geospatial data and argument file I/O routes through GDAL.
No NetCDF, HDF5, or shapefile reading outside of GDAL or the dedicated thread-safe wrappers.

### 8.1 `GeometryFactory` extended API (`include/spatial/io/geometryfactory.h`)

```cpp
namespace HydroCouple::Spatial {

class HYDROCOUPLESDK_EXPORT GeometryFactory {
public:
    // ── OGR ↔ SDK geometry conversion (all string params: std::string) ────
    static OGRGeometry *exportToOGRGeometry(const IGeometry *g);
    static HydroCouple::Spatial::Geometry *importFromOGRGeometry(const OGRGeometry *g);
    static Geometry    *importFromWkt(const std::string &wkt);
    static Geometry    *importFromWkb(const unsigned char *wkb, int nBytes = -1);

    // ── Vector file I/O ──────────────────────────────────────────────────
    static Network           *importNetworkFromLayer(OGRLayer *layer,
                                                     const std::string &idField = "");
    static bool               exportNetworkToLayer(const Network *net, OGRLayer *layer,
                                                   std::string &err);
    static TIN               *importTINFromLayer(OGRLayer *layer);
    static PolyhedralSurface *importPolyhedralSurfaceFromLayer(OGRLayer *layer);
    static bool               exportTINToLayer(const TIN *tin, OGRLayer *layer,
                                               std::string &err);

    // ── Raster I/O ───────────────────────────────────────────────────────
    static Raster       *importRasterFromDataset(GDALDataset *ds);
    static RegularGrid2D *importGrid2DFromDataset(GDALDataset *ds);

    // ── Generic entry point ───────────────────────────────────────────────
    // GDALOpenEx dispatch: detects vector vs raster, returns appropriate object
    static Geometry     *importFromFile(const std::string &filePath,
                                        std::string &errorMessage);

    // ── Argument I/O via GDAL ─────────────────────────────────────────────
    // Reads a GDAL-supported file and populates an IArgument.
    // Handles: shapefiles, GeoTIFF, GeoPackage, GML, NetCDF, HDF5 (via GDAL).
    static bool initArgumentFromFile(HydroCouple::IArgument *arg,
                                     const std::string &filePath,
                                     std::string &errorMessage);

    // ── Write helpers ─────────────────────────────────────────────────────
    static bool writeGeometryToFile(const GeometryComponentDataItem<double> *item,
                                    const std::string &fieldName,
                                    const std::string &driverName,
                                    const std::string &outputFile,
                                    std::string &errorMessage);

    static bool writeTINToFile(const TIN *tin,
                               const std::string &driverName,
                               const std::string &outputFile,
                               std::string &errorMessage);

    static void registerGDAL();

private:
    static bool m_GDALRegistered;
};

} // namespace HydroCouple::Spatial
```

### 8.2 `initArgumentFromFile` — Implementation Pattern

```
1. GDALOpenEx(filePath, GDAL_OF_READONLY | GDAL_OF_VERBOSE_ERROR)
2. If GetRasterCount() > 0:
     → RasterIO into a std::vector<double>
     → arg->initialize(data as hydrocouple_variant, ArgumentInputType::MEMORY_OBJECT, msg)
3. Else if GetLayerCount() > 0:
     → Iterate OGRFeature; extract field values or geometry as hydrocouple_variant
     → arg->initialize(...)
4. GDALClose()
```

---

## 9. Thread-Safe I/O Wrappers

### 9.1 NetCDF — `include/io/netcdf/` — namespace `HydroCouple::IO::NetCDF`

- Replace all `#pragma omp critical` guards with `std::shared_mutex` (one static per pool):
  - `std::shared_lock` for reads (`getVar`, `getAtt`, `getDim`)
  - `std::unique_lock` for writes (`addVar`, `addDim`, `putVar`, `sync`, `open`, `close`)
- Add `nchydrocouple.h`: time-series and argument persistence utilities

### 9.2 HDF5 — `include/io/hdf5/` — namespace `HydroCouple::IO::HDF5` (new)

| File | Wraps |
|---|---|
| `threadsafeh5file.h` | `hid_t` file — `H5Fcreate/H5Fopen/H5Fclose` |
| `threadsafeh5group.h` | `hid_t` group |
| `threadsafeh5dataset.h` | `hid_t` dataset — hyperslab read/write; `template<T> read/write(vector<T>&)` |
| `threadsafeh5attribute.h` | `hid_t` attribute |
| `h5hydrocouple.h` | Argument persistence + time-series utilities |

---

## 10. Missing Interface Implementations

| New class | Interface | Namespace |
|---|---|---|
| `WorkflowComponent` | `IWorkflowComponent` | `HydroCouple` |
| `WorkflowComponentInfo` | `IWorkflowComponentInfo` | `HydroCouple` |
| `WorkflowStatusChangeEventArgs` | `IWorkflowComponentStatusChangeEventArgs` | `HydroCouple` |
| `CloneableModelComponent` | `ICloneableModelComponent` | `HydroCouple` |
| `Raster` / `RasterBand` | `IRaster` / `IRasterBand` | `HydroCouple::Spatial` |
| `RegularGrid2D` | `IRegularGrid2D` | `HydroCouple::Spatial` |
| `RegularGrid3D` | `IRegularGrid3D` | `HydroCouple::Spatial` |
| `NetworkComponentDataItem<T>` | `INetworkComponentDataItem` | `HydroCouple::Spatial` |
| `PolyhedralSurfaceComponentDataItem<T>` | `IPolyhedralSurfaceComponentDataItem` | `HydroCouple::Spatial` |
| `RasterComponentDataItem<T>` | `IRasterComponentDataItem` | `HydroCouple::Spatial` |
| `RegularGrid2DComponentDataItem<T>` | `IRegularGrid2DComponentDataItem` | `HydroCouple::Spatial` |
| `RegularGrid3DComponentDataItem<T>` | `IRegularGrid3DComponentDataItem` | `HydroCouple::Spatial` |
| `TimeSeriesPolyhedralSurfaceComponentDataItem<T>` | `ITimeSeriesPolyhedralSurfaceComponentDataItem` | `HydroCouple::SpatioTemporal` |
| `TimeSeriesTINComponentDataItem<T>` | `ITimeSeriesTINComponentDataItem` | `HydroCouple::SpatioTemporal` |
| `TimeSeriesRasterComponentDataItem<T>` | `ITimeSeriesRasterComponentDataItem` | `HydroCouple::SpatioTemporal` |
| `TimeRegularGrid2DComponentDataItem<T>` | `ITimeRegularGrid2DComponentDataItem` | `HydroCouple::SpatioTemporal` |
| `TimeRegularGrid3DComponentDataItem<T>` | `ITimeRegularGrid3DComponentDataItem` | `HydroCouple::SpatioTemporal` |
| `TimeNetworkComponentDataItem<T>` | `ITimeNetworkComponentDataItem` | `HydroCouple::SpatioTemporal` |

---

## 11. Vectorisation Strategy

All vectorisation uses **portable mechanisms only** — no platform SIMD intrinsics.

### 11.1 `#pragma omp simd`

Used wherever the loop body is a simple arithmetic expression over contiguous arrays.
Always guarded with `#ifdef _OPENMP` so the code compiles and runs correctly without OpenMP.

### 11.2 `std::copy` for contiguous block transfers

Bulk `getValues` / `setValues` on contiguous ranges use `std::copy`, which compilers
(GCC, Clang, MSVC) optimise to `memcpy` or vectorised load-store instructions automatically.

### 11.3 SoA coordinate layout (primary enabler)

The `VertexPool` SoA layout is the single most impactful change for vectorisation.
Operations that previously required chasing `HCVertex*` pointers now operate on flat
`double[]` arrays, which the auto-vectoriser can process in 256-bit or 512-bit chunks.

### 11.4 `Vect` stack allocation

Eliminates heap allocation per vector operation — keeps normal vectors in L1 cache.

### 11.5 Portable compiler optimisation flags

```cmake
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-ftree-vectorize" HAS_TREE_VEC)
if(HAS_TREE_VEC)
    target_compile_options(HydroCoupleSDK PRIVATE $<$<CONFIG:Release>:-ftree-vectorize>)
endif()
target_compile_options(HydroCoupleSDK PRIVATE
    $<$<CONFIG:Release>:$<IF:$<CXX_COMPILER_ID:MSVC>,/O2,-O3>>)
# No -march=native or /arch:AVX2 — preserves portability across CPU generations
```

---

## 12. Google Test Unit Tests

Test files mirror the source tree exactly. Every test class has `_Test` suffix and
uses `TEST_F` (fixture) for anything that requires a mesh or component setup.

```
tests/
├── hydrocouple/
│   ├── test_signal.cpp            — connect/emit/disconnect/block; expired slot cleanup
│   ├── test_description.cpp       — property change signals; caption/description
│   ├── test_identity.cpp          — UUID uniqueness; id round-trip
│   ├── test_valuedefinition.cpp   — hydrocouple_variant; Quality/Quantity
│   ├── test_argument.cpp          — XML/file round-trip; optional/readOnly
│   ├── test_exchangeitems.cpp     — consumer/provider wiring; updateValues dispatch
│   └── test_workflow.cpp          — WorkflowComponent lifecycle; status transitions
├── temporal/
│   ├── test_datetime.cpp          — Julian Day conversions; modifiedJulianDay; serialDate
│   ├── test_timeseries.cpp        — CSV load; interpolation; concurrent reads
│   └── test_timeseriesexchangeitem.cpp
├── spatial/
│   ├── test_point.cpp             — coordinates; WKT/WKB round-trips
│   ├── test_linestring_soa.cpp    — SoA layout correctness; SIMD length vs scalar
│   ├── test_polygon.cpp           — area; containment
│   ├── test_quadedgepool.cpp      — allocate/free; splice invariants; orbit traversal
│   ├── test_vertexpool.cpp        — bulkGetXYZ; bulkDistSq; SoA correctness
│   ├── test_polyhedralsurface.cpp — topology construction; vertex/patch counts
│   ├── test_tin.cpp               — triangulation; triangle area; normal
│   ├── test_network.cpp           — directed graph; edge angular ordering
│   ├── test_raster.cpp            — GDAL open; band read/write; nodata
│   ├── test_regulargrid2d.cpp     — Cartesian/curvilinear construction; bulk getValue
│   └── test_geometryfactory.cpp   — GDAL import/export; TIN from shapefile
├── spatiotemporal/
│   ├── test_timegeometry.cpp
│   └── test_timeregulargrid.cpp   — parallel getValues correctness vs serial
└── io/
    ├── test_netcdf_rw.cpp         — round-trip; shared_mutex read concurrency
    ├── test_netcdf_threadsafe.cpp — N=16 threads; no corruption
    ├── test_hdf5_rw.cpp
    └── test_hdf5_threadsafe.cpp
```

**Topology invariant test pattern:**
```cpp
TEST_F(QuadEdgePoolTest, SplicePreservesOrbit) {
    auto base = pool.allocateQuad();
    // after splice(a, b), traverseOrigOrbit(a) must return to a
    auto orbit = traverseOrbit(pool, base);
    EXPECT_EQ(orbit.back(), base);
    EXPECT_EQ(orbit.size(), expectedOrbitLength);
}
```

---

## 13. Doxygen Documentation

`docs/Doxyfile.in` mirrors the HydroCouple interface project's Doxyfile exactly
with these substitutions:

```doxyfile
PROJECT_NAME           = "@PROJECT_NAME@"
PROJECT_NUMBER         = "@PROJECT_VERSION@"
PROJECT_BRIEF          = "HydroCouple C++ Modeling Framework SDK"
OUTPUT_DIRECTORY       = @CMAKE_CURRENT_BINARY_DIR@/doxygen
INPUT                  = @CMAKE_SOURCE_DIR@/include @CMAKE_SOURCE_DIR@/src @CMAKE_SOURCE_DIR@/README.md
RECURSIVE              = YES
EXTRACT_ALL            = YES
EXTRACT_PRIVATE        = YES
GENERATE_HTML          = YES
IGNORE_PREFIX          = I Abstract
WARN_IF_UNDOCUMENTED   = YES
USE_MDFILE_AS_MAINPAGE = @CMAKE_SOURCE_DIR@/README.md
ALIASES               += "threadsafety=\par Thread Safety\n"
```

Every class, method, parameter, and return value has a full Doxygen block.
Every file has `\file`, `\author`, `\version`, `\date`, `\license` header.

---

## 14. Implementation Order

| Phase | Description | Files |
|---|---|---|
| 1 | vcpkg.json, vcpkg-configuration.json, CMakePresets.json, version.h.in | 4 new |
| 2 | hydrocouplesdk.h, signal.h, uuid.h, hydrocoupleforward.h, stdafx.h | 5 modified/new |
| 3 | CMakeLists.txt hierarchy (top-level, src/, tests/, docs/) | 4 new |
| 4a | Rename include/core/ → include/hydrocouple/; wrap in `namespace HydroCouple` | ~35 modified |
| 4b | Qt→C++17 substitution (bottom-up: description→identity→valuedefinition→…) | ~40 modified |
| 5 | Vect stack alloc; QuadEdgePool; VertexPool; FacePool | 7 new |
| 6 | Edge/Vertex handle refactor | edge.h, point.h + .cpp |
| 7 | PolyhedralSurface/TIN refactor with pools | polyhedralsurface.h/cpp, tin.cpp |
| 8 | Network refactor with pools | network.h/cpp |
| 9 | LineString SoA | linestring.h/cpp |
| 10 | RegularGrid2D/3D new | 4 new files |
| 11 | All component data items (flat arrays, bulk get/set) | ~12 new/modified |
| 12 | Raster, WorkflowComponent, CloneableModelComponent (missing impls) | ~8 new |
| 13 | GeometryFactory GDAL I/O extensions | geometryfactory.h/cpp |
| 14 | NetCDF shared_mutex upgrade + nchydrocouple.h | 6 modified + 1 new |
| 15 | HDF5 wrappers (new) | 9 new |
| 16 | GTest suite | ~28 new test files |
| 17 | Doxygen Doxyfile.in + docs/CMakeLists.txt | 2 new |
| 18 | Module aggregate headers + umbrella HydroCoupleSDK.h | ~9 new |

**Critical path:** Phase 2 → `description.h` → `identity.h` → `valuedefinition.h`
→ `componentdataitem.h` → `modelcomponent.h` → everything else.

---

## 15. Verification Checklist

1. `cmake --preset macos -DBUILD_TESTS=ON` — clean configure; vcpkg resolves all deps
2. `cmake --build build --parallel` — zero warnings with `-Wall -Wextra`
3. `ctest --test-dir build --output-on-failure` — all GTest cases pass
4. Repeat with `USE_NETCDF=OFF USE_HDF5=OFF` — conditional code compiles cleanly
5. `QuadEdgePool` orbit invariant: `traverseOrbit(splice(a,b))` returns correct orbit
6. SoA equivalence: `VertexPool::bulkGetXYZ` result matches `vertex(i)->x()` for all i
7. Thread-safety stress: N=16 threads writing to separate NC/H5 vars — no corruption
8. `cmake --build build --target docs` — zero undocumented-symbol Doxygen warnings
9. `cmake --install build --prefix /tmp/hcsdk_test` — headers + lib at expected paths

---

## 16. C++20 Upgrade

Replace `set(CMAKE_CXX_STANDARD 17)` with `set(CMAKE_CXX_STANDARD 20)` everywhere.
Minimum CMake version stays at 3.18 (full C++20 support landed in CMake 3.12+).
All compilers in the vcpkg toolchain support C++20: MSVC ≥ 19.29, GCC ≥ 11, Clang ≥ 13,
AppleClang ≥ 13 (Xcode 13).

### 16.1 C++20 Features Adopted Across the SDK

| Feature | Where used |
|---|---|
| `std::span<T>` | All bulk data APIs — zero-copy view of caller-owned arrays |
| `std::string_view` | All read-only string parameters (replaces `const std::string&`) |
| Concepts (`requires`) | `IMeshGenerator`, `IInterpolator`, `TerrainProvider` constraints |
| Ranges (`std::ranges::`) | Container algorithms (sort, find, transform) throughout |
| Designated initialisers | All options structs (`RectilinearOptions{.nx=10, .ny=20}`) |
| `std::format` | Error messages, log strings (replaces `std::ostringstream`) |
| `std::jthread` + `std::stop_token` | Cancellable async mesh generation |
| `std::atomic_ref<T>` | Progress reporting from worker threads |
| Three-way comparison (`<=>`) | Ordering for `MeshVertex`, `GridIndex` value types |
| `[[nodiscard]]` | All `generate()`, `interpolate()`, `sample()` return values |
| `std::numbers::pi` | Geometry angle calculations |

### 16.2 `std::span` replaces vector copies in bulk APIs

```cpp
// Before (C++17 — copies the data into a vector)
std::vector<double> sampleBulk(const std::vector<HydroCouple::Spatial::Point> &pts) const;

// After (C++20 — zero-copy view)
std::vector<double> sampleBulk(std::span<const HydroCouple::Spatial::Point> pts) const;
```

### 16.3 Concepts for generator/interpolator constraints

```cpp
namespace HydroCouple::Tools {

template<typename T>
concept MeshGeneratorOptions = requires(T t) {
    { t.maxElementArea } -> std::convertible_to<double>;
    { t.minAngleDeg    } -> std::convertible_to<double>;
};

template<typename T>
concept InterpolatorMethod = requires(T t,
    std::span<const double> srcVals,
    std::span<const double> weights,
    double &outVal) {
    { t.interpolate(srcVals, weights, outVal) } -> std::same_as<bool>;
};

} // namespace HydroCouple::Tools
```

---

## 17. `HydroCouple::Tools` — Namespace Design

### 17.1 Overview

The `Tools` module lives in `include/tools/` and `src/tools/`, namespace `HydroCouple::Tools`.
It is a **separate optional library target** (`HydroCoupleTools`) that links against
`HydroCoupleSDK` and GDAL but not against NetCDF/HDF5 (those stay in `IO`).
Add it to CMakeLists with `option(BUILD_TOOLS "Build HydroCouple tools" ON)`.

Sub-namespaces:

| Namespace | Folder | Responsibility |
|---|---|---|
| `HydroCouple::Tools` | `include/tools/` | Domain spec; common value types |
| `HydroCouple::Tools::Mesh` | `include/tools/mesh/` | Mesh generation |
| `HydroCouple::Tools::Interpolation` | `include/tools/interpolation/` | Spatial/temporal interpolation |
| `HydroCouple::Tools::Terrain` | `include/tools/terrain/` | DTM sampling and thinning |

### 17.2 Header Layout

```
include/tools/
├── tools.h                           ← module aggregate
│
├── domain/
│   ├── terraindomain.h               ← TerrainDomain — the universal mesh input spec
│   ├── constraintsegment.h           ← ConstraintSegment
│   ├── regionmarker.h                ← RegionMarker
│   └── domainattribute.h             ← DomainAttribute<T>
│
├── mesh/                             ← namespace HydroCouple::Tools::Mesh
│   ├── mesh.h                        ← sub-module aggregate
│   ├── imeshgenerator.h              ← IMeshGenerator concept + abstract base
│   ├── meshresult.h                  ← MeshResult (SDK-native types)
│   │
│   ├── triangulation/
│   │   ├── delaunaymeshgenerator.h   ← DelaunayMeshGenerator (Triangle lib, from openswmm.gui)
│   │   └── triangulationoptions.h    ← TriangulationOptions
│   │
│   ├── rectilinear/
│   │   ├── rectilineargrid2dgenerator.h
│   │   ├── rectilineargrid3dgenerator.h
│   │   └── rectilinearoptions.h
│   │
│   ├── curvilinear/
│   │   ├── curvilineargrid2dgenerator.h
│   │   ├── curvilineargrid3dgenerator.h
│   │   └── curvilinearoptions.h
│   │
│   └── unstructured/
│       ├── unstructuredmeshgenerator.h   ← general advancing-front / Delaunay refine
│       └── unstructuredmeshoptions.h
│
├── interpolation/                    ← namespace HydroCouple::Tools::Interpolation
│   ├── interpolation.h               ← sub-module aggregate
│   ├── iinterpolator.h               ← IInterpolator concept
│   │
│   ├── methods/
│   │   ├── bilinearinterpolator.h        ← bilinear on structured grids
│   │   ├── barycentricinterpolator.h     ← linear barycentric on triangles
│   │   ├── idwinterpolator.h             ← inverse distance weighting
│   │   ├── nearestneighborinterpolator.h
│   │   └── naturalneighborinterpolator.h ← Sibson natural neighbour
│   │
│   └── spatial/
│       ├── meshtomeshinterpolator.h      ← TIN/unstructured ↔ TIN/unstructured
│       ├── gridtomeshinterpolator.h      ← RegularGrid2D → mesh vertices
│       ├── meshtogridinterpolator.h      ← mesh values → RegularGrid2D
│       └── gridtogridinterpolator.h      ← RegularGrid → RegularGrid (resampling)
│
└── terrain/                          ← namespace HydroCouple::Tools::Terrain
    ├── terrain.h                     ← sub-module aggregate
    ├── terrainsampler.h              ← TerrainSampler (bilinear DTM sampler, Qt-free)
    ├── terrainthinner.h              ← TerrainThinner (normal-deviation decimation)
    └── terrainanalysis.h             ← slope, aspect, curvature (GDAL-based)
```

### 17.3 `TerrainDomain` — Universal Mesh Input Specification

```cpp
namespace HydroCouple::Tools {

/*!
 * \brief Complete specification of a 2D meshing domain.
 *
 * Encapsulates all geometric inputs (boundary, holes, constraints, regions)
 * and domain-level attributes. Passed to any IMeshGenerator::generate().
 * Immutable once built; use TerrainDomain::Builder to construct.
 */
class HYDROCOUPLESDK_EXPORT TerrainDomain {
public:
    // ── Nested types ───────────────────────────────────────────────────────
    using Ring      = std::vector<HydroCouple::Spatial::Point>;
    using Attribute = std::pair<std::string, HydroCouple::hydrocouple_variant>;

    // ── Builder (C++20 designated-initialiser friendly) ───────────────────
    struct Spec {
        std::vector<Ring>              outerBoundaries;   ///< One or more closed rings.
        std::vector<Ring>              holes;             ///< Interior rings (excluded).
        std::vector<ConstraintSegment> constraints;       ///< Edges forced into mesh.
        std::vector<RegionMarker>      regions;           ///< Seed → region attribute.
        std::vector<Attribute>         domainAttributes;  ///< Arbitrary metadata.
        std::string                    crsWkt;            ///< Coordinate reference system.
        double                         simplifyEpsilon = 0.0; ///< RDP tolerance (0 = none).
        double                         snapEpsilon     = 0.0; ///< Steiner snap radius.
    };

    explicit TerrainDomain(Spec spec);

    // ── Accessors ─────────────────────────────────────────────────────────
    std::span<const Ring>              outerBoundaries()  const noexcept;
    std::span<const Ring>              holes()            const noexcept;
    std::span<const ConstraintSegment> constraints()      const noexcept;
    std::span<const RegionMarker>      regions()          const noexcept;
    std::span<const Attribute>         domainAttributes() const noexcept;
    std::string_view                   crsWkt()           const noexcept;

    bool isEmpty() const noexcept;

    // ── Factory — load from GDAL vector layer ─────────────────────────────
    static TerrainDomain fromOGRLayer(OGRLayer *layer, std::string &errorMsg);

    // ── Derived geometry ──────────────────────────────────────────────────
    HydroCouple::Spatial::Envelope envelope() const;

private:
    Spec m_spec;
};

} // namespace HydroCouple::Tools
```

### 17.4 `ConstraintSegment` and `RegionMarker`

```cpp
namespace HydroCouple::Tools {

/*!
 * \brief A polyline that must appear as constrained edges in the mesh.
 * Consecutive point pairs become Triangle segments (marker preserved on output edges).
 */
struct ConstraintSegment {
    std::vector<HydroCouple::Spatial::Point> path;
    int         marker  = 0;    ///< Integer tag; preserved in output edges.
    std::string tag;            ///< Human-readable label (resolved from marker).
};

/*!
 * \brief A seed point inside a sub-region; carries an attribute Triangle
 *        propagates into all triangles of that region.
 */
struct RegionMarker {
    HydroCouple::Spatial::Point position;
    double      attribute = 0.0;   ///< Region id (mapped back to tag string).
    double      maxArea   = -1.0;  ///< Area cap for this region (-1 = global).
    std::string tag;
};

} // namespace HydroCouple::Tools
```

### 17.5 `MeshResult` — SDK-Native Output

```cpp
namespace HydroCouple::Tools::Mesh {

struct MeshVertex {
    HydroCouple::Spatial::Point position; ///< x, y (z filled post-generation from terrain).
    double      z      = 0.0;
    int         marker = 0;
    std::string tag;

    auto operator<=>(const MeshVertex &) const = default;  // C++20
};

struct MeshElement {          ///< Triangle (nNodes=3) or quad (nNodes=4)
    std::array<int, 4> nodeIndices = {-1,-1,-1,-1};
    int  nNodes   = 3;
    double attribute = 0.0;   ///< Region attribute propagated by Triangle.
    std::string tag;
};

struct MeshEdge {
    int  v0 = -1, v1 = -1;
    int  marker = 0;
    std::string tag;
};

struct MeshResult {
    std::vector<MeshVertex>  vertices;
    std::vector<MeshElement> elements;
    std::vector<MeshEdge>    edges;
    bool        ok       = false;
    std::string errorMsg;
};

} // namespace HydroCouple::Tools::Mesh
```

### 17.6 `IMeshGenerator` — C++20 Concept-Constrained Interface

```cpp
namespace HydroCouple::Tools::Mesh {

/*!
 * \brief Concept satisfied by any mesh generator options struct.
 * Requires at minimum a max-element area and a min-angle quality parameter.
 */
template<typename T>
concept GeneratorOptions = requires(T t) {
    { t.maxElementArea } -> std::convertible_to<double>;
    { t.minAngleDeg    } -> std::convertible_to<double>;
};

/*!
 * \brief Abstract base for all mesh generators.
 * Subclasses override generate() with their type-erased options struct.
 */
class HYDROCOUPLESDK_EXPORT IMeshGenerator {
public:
    virtual ~IMeshGenerator() = default;

    /*!
     * \brief Generate a mesh from \p domain.
     * \param domain    The complete domain specification.
     * \param stopToken C++20 stop token for cooperative cancellation.
     * \return          MeshResult with ok=false and errorMsg on failure.
     */
    [[nodiscard]] virtual MeshResult generate(
        const HydroCouple::Tools::TerrainDomain &domain,
        std::stop_token stopToken = {}) const = 0;

    /*!
     * \brief Atomic progress value in [0.0, 1.0]; updated during long runs.
     * Caller may read this from any thread.
     */
    [[nodiscard]] double progress() const noexcept {
        return m_progress.load(std::memory_order_relaxed);
    }

protected:
    mutable std::atomic<double> m_progress{0.0};
};

} // namespace HydroCouple::Tools::Mesh
```

### 17.7 `DelaunayMeshGenerator` — Constrained-Delaunay Triangulation

Ported directly from `openswmm.gui/include/mesh/meshgenerator.h`, Qt types replaced:

```cpp
namespace HydroCouple::Tools::Mesh {

/*! \brief Quality parameters for the constrained-Delaunay triangulator. */
struct TriangulationOptions {
    double maxElementArea   = 0.0;    ///< 0 = no global cap.
    double minAngleDeg      = 28.0;   ///< 0..34 reliable; >34 may not terminate.
    bool   allowSteiner     = true;   ///< false = no Steiner points on boundary (-YY).
    bool   conformingDelaunay = false;///< -D switch (full vs. constrained Delaunay).
    int    maxSteinerPoints = -1;     ///< -SN; -1 = unlimited.
    bool   quiet            = true;   ///< -Q suppress Triangle stderr.
    std::string customSwitchString;   ///< Overrides all above when non-empty.
};

static_assert(GeneratorOptions<TriangulationOptions>);  // C++20 concept check

/*!
 * \brief 2D constrained-Delaunay triangulator wrapping Shewchuk's Triangle library.
 *
 * Accepts a TerrainDomain (boundary rings, holes, constraint segments, region
 * markers) and Triangle quality options.  Tags propagate from input to output
 * via Triangle's integer marker mechanism.
 *
 * Usage:
 * \code
 *     using namespace HydroCouple::Tools;
 *     TerrainDomain domain(TerrainDomain::Spec{
 *         .outerBoundaries = {ring},
 *         .constraints     = {conduitSeg},
 *         .regions         = {subCatchmentMarker},
 *     });
 *     Mesh::DelaunayMeshGenerator gen;
 *     gen.setOptions({.maxElementArea = 50.0, .minAngleDeg = 28.0});
 *     const auto result = gen.generate(domain);
 * \endcode
 */
class HYDROCOUPLESDK_EXPORT DelaunayMeshGenerator : public IMeshGenerator {
public:
    explicit DelaunayMeshGenerator(TriangulationOptions opts = {});

    void setOptions(const TriangulationOptions &opts);
    [[nodiscard]] const TriangulationOptions &options() const noexcept;

    [[nodiscard]] MeshResult generate(
        const HydroCouple::Tools::TerrainDomain &domain,
        std::stop_token stopToken = {}) const override;

    /*!
     * \brief Resolve a vertex marker integer back to its tag string.
     * Only valid after generate() has been called; built from input SteinerPoints.
     */
    [[nodiscard]] std::string tagForVertexMarker(int marker) const;
    [[nodiscard]] std::string tagForEdgeMarker(int marker) const;

private:
    TriangulationOptions m_opts;
    mutable std::unordered_map<int, std::string> m_vertexTagByMarker;
    mutable std::unordered_map<int, std::string> m_edgeTagByMarker;
    mutable std::unordered_map<int, std::string> m_triangleTagByRegionId;
};

} // namespace HydroCouple::Tools::Mesh
```

### 17.8 Rectilinear Mesh Generators

```cpp
namespace HydroCouple::Tools::Mesh {

struct RectilinearOptions {
    double maxElementArea = 0.0;    ///< ignored for uniform grids; used for refinement
    double minAngleDeg    = 90.0;   ///< always 90 for rectilinear
    int    nx = 10, ny = 10;        ///< cell counts
    int    nz = 1;                  ///< layers (3D only)
    double dx = 0.0, dy = 0.0, dz = 0.0; ///< spacing (0 = derive from domain + n)
};

static_assert(GeneratorOptions<RectilinearOptions>);

/*!
 * \brief Generates an axis-aligned rectilinear quad mesh from domain extent.
 * The domain's outer boundary bounding box defines the grid extent.
 * Cells outside the outer boundary polygon (or inside holes) are marked inactive.
 */
class HYDROCOUPLESDK_EXPORT RectilinearGrid2DGenerator : public IMeshGenerator {
public:
    explicit RectilinearGrid2DGenerator(RectilinearOptions opts = {});
    void setOptions(const RectilinearOptions &opts);

    [[nodiscard]] MeshResult generate(
        const TerrainDomain &domain,
        std::stop_token stopToken = {}) const override;

    /*!
     * \brief Also return a RegularGrid2D object populated from the result.
     * Convenience for callers that need the grid as an IRegularGrid2D.
     */
    [[nodiscard]] std::unique_ptr<HydroCouple::Spatial::RegularGrid2D>
    generateGrid(const TerrainDomain &domain) const;

private:
    RectilinearOptions m_opts;
};

/*! \brief Generates a hexahedral rectilinear 3D mesh (layers of quads). */
class HYDROCOUPLESDK_EXPORT RectilinearGrid3DGenerator : public IMeshGenerator {
public:
    explicit RectilinearGrid3DGenerator(RectilinearOptions opts = {});
    void setOptions(const RectilinearOptions &opts);

    [[nodiscard]] MeshResult generate(
        const TerrainDomain &domain,
        std::stop_token stopToken = {}) const override;

private:
    RectilinearOptions m_opts;
};

} // namespace HydroCouple::Tools::Mesh
```

### 17.9 Curvilinear Mesh Generators

```cpp
namespace HydroCouple::Tools::Mesh {

struct CurvilinearOptions {
    double maxElementArea = 0.0;
    double minAngleDeg    = 60.0;
    int    ni = 10, nj = 10;    ///< grid lines in i and j directions
    int    nk = 1;              ///< layers (3D)
    /*!
     * \brief Optional control-point grid for algebraic transfinite interpolation.
     * If empty, TFI is derived directly from the domain boundary.
     */
    std::vector<HydroCouple::Spatial::Point> controlPoints;
};

static_assert(GeneratorOptions<CurvilinearOptions>);

/*!
 * \brief Generates a body-fitted curvilinear structured mesh using
 *        Transfinite Interpolation (TFI) from domain boundary curves.
 *
 * The domain must have exactly 4 boundary segments (i-min, j-min, i-max, j-max)
 * or the generator subdivides the boundary automatically using corner detection.
 */
class HYDROCOUPLESDK_EXPORT CurvilinearGrid2DGenerator : public IMeshGenerator {
public:
    explicit CurvilinearGrid2DGenerator(CurvilinearOptions opts = {});
    void setOptions(const CurvilinearOptions &opts);

    [[nodiscard]] MeshResult generate(
        const TerrainDomain &domain,
        std::stop_token stopToken = {}) const override;

private:
    CurvilinearOptions m_opts;
    // TFI helpers
    static std::vector<HydroCouple::Spatial::Point>
    transfiniteInterpolate(std::span<const HydroCouple::Spatial::Point> c0,
                           std::span<const HydroCouple::Spatial::Point> c1,
                           std::span<const HydroCouple::Spatial::Point> d0,
                           std::span<const HydroCouple::Spatial::Point> d1,
                           int ni, int nj);
};

/*! \brief 3D curvilinear mesh — layers of curvilinear 2D slabs. */
class HYDROCOUPLESDK_EXPORT CurvilinearGrid3DGenerator : public IMeshGenerator {
public:
    explicit CurvilinearGrid3DGenerator(CurvilinearOptions opts = {});
    void setOptions(const CurvilinearOptions &opts);
    [[nodiscard]] MeshResult generate(
        const TerrainDomain &domain,
        std::stop_token stopToken = {}) const override;
private:
    CurvilinearOptions m_opts;
};

} // namespace HydroCouple::Tools::Mesh
```

### 17.10 Terrain Processing

Ported from `openswmm.gui` with Qt removed:

```cpp
namespace HydroCouple::Tools::Terrain {

/*! \brief Parameters for normal-deviation terrain thinning. */
struct ThinnerOptions {
    double gridSpacing        = 0.0;   ///< 0 = native pixel size.
    double normalDotThreshold = 0.95;  ///< cos θ cutoff; 0.95 ≈ 18°.
    bool   useAverageDot      = false; ///< false = min; true = average fan dot.
    int    maxPoints          = 0;     ///< Hard cap. 0 = unlimited.
    int    maxIterations      = 0;     ///< 0 = until convergence.
    bool   useMinSpacing      = false;
    double minSpacing         = 0.0;   ///< 0 = 2×pixelSize.
};

/*!
 * \brief Bilinear elevation sampler backed by a GDAL raster band.
 * Returns NaN for out-of-bounds or NoData pixels.
 */
class HYDROCOUPLESDK_EXPORT TerrainSampler {
public:
    TerrainSampler();
    ~TerrainSampler();
    TerrainSampler(const TerrainSampler &) = delete;
    TerrainSampler &operator=(const TerrainSampler &) = delete;

    bool open(std::string_view filePath, int band = 1);
    void close();
    [[nodiscard]] bool isOpen()   const noexcept;
    [[nodiscard]] std::string crsWkt()   const;
    [[nodiscard]] std::string errorMsg() const;

    [[nodiscard]] double sample(double x, double y) const;
    [[nodiscard]] std::vector<double> sampleBulk(
        std::span<const HydroCouple::Spatial::Point> pts) const;

private:
    GDALDataset *m_ds        = nullptr;
    int          m_band      = 1;
    double       m_geo[6]    = {};
    double       m_invGeo[6] = {};
    int          m_w = 0, m_h = 0;
    double       m_noData    = 0.0;
    bool         m_hasNoData = false;
    mutable std::string m_errorMsg;
};

/*!
 * \brief Terrain-adaptive Steiner point selector using iterative
 *        normal-deviation decimation.
 *
 * Ported from openswmm.gui/DTMThinner with Qt types replaced by STL+GDAL.
 * Algorithm: iterative normal-deviation decimation (see dtmthinner.h in openswmm.gui).
 */
class HYDROCOUPLESDK_EXPORT TerrainThinner {
public:
    TerrainThinner();
    ~TerrainThinner();
    TerrainThinner(const TerrainThinner &) = delete;
    TerrainThinner &operator=(const TerrainThinner &) = delete;

    bool open(std::string_view filePath, int band = 1);
    void close();
    [[nodiscard]] bool isOpen()   const noexcept;
    [[nodiscard]] double pixelSize() const;
    [[nodiscard]] std::string crsWkt()   const;
    [[nodiscard]] std::string errorMsg() const;

    /*!
     * \brief Generate terrain-significant Steiner points within \p domain.
     * \param domain  Meshing extent (same CRS as the DTM raster).
     * \param opts    Thinning parameters.
     * \param outZ    If non-null, filled in parallel with each point's elevation.
     *                These values are exact — do NOT re-sample them later.
     * \return        (x, y) coordinates of terrain-feature vertices.
     */
    [[nodiscard]] std::vector<HydroCouple::Spatial::Point>
    generatePoints(const HydroCouple::Tools::TerrainDomain &domain,
                   const ThinnerOptions &opts = {},
                   std::vector<double> *outZ  = nullptr) const;

    [[nodiscard]] double sampleAt(double x, double y) const;

    void readPixels(const HydroCouple::Spatial::Envelope &bbox,
                    std::vector<HydroCouple::Spatial::Point> &xyOut,
                    std::vector<double>                      &zOut) const;

private:
    GDALDataset *m_ds        = nullptr;
    int          m_band      = 1;
    double       m_geo[6]    = {};
    double       m_invGeo[6] = {};
    int          m_w = 0, m_h = 0;
    double       m_noData    = 0.0;
    bool         m_hasNoData = false;
    mutable std::string m_errorMsg;
};

} // namespace HydroCouple::Tools::Terrain
```

### 17.11 Interpolation

```cpp
namespace HydroCouple::Tools::Interpolation {

/*!
 * \brief Concept satisfied by any spatial interpolation method class.
 */
template<typename T>
concept InterpolatorMethod = requires(T t,
    std::span<const double> srcValues,
    std::span<const double> weights) {
    { t.apply(srcValues, weights) } -> std::convertible_to<double>;
};

// ── Method policies (satisfy InterpolatorMethod concept) ──────────────────

struct BilinearMethod {
    double apply(std::span<const double> vals,
                 std::span<const double> weights) const noexcept;
};

struct BarycentricMethod {
    double apply(std::span<const double> vals,          ///< 3 values (triangle nodes)
                 std::span<const double> weights) const noexcept; ///< barycentric coords
};

struct IDWMethod {
    int    power     = 2;
    double maxRadius = std::numeric_limits<double>::max();

    double apply(std::span<const double> vals,
                 std::span<const double> invDistPowers) const noexcept;
};

struct NearestNeighborMethod {
    double apply(std::span<const double> vals,
                 std::span<const double>) const noexcept;
};

// ── Spatial interpolators ─────────────────────────────────────────────────

/*!
 * \brief Interpolates values from a source mesh/TIN to target query points.
 *
 * Locates the enclosing triangle for each target point via the source TIN's
 * Octree index, then applies the chosen method (default: barycentric).
 */
class HYDROCOUPLESDK_EXPORT MeshToMeshInterpolator {
public:
    explicit MeshToMeshInterpolator(
        const HydroCouple::Spatial::TIN *sourceMesh,
        BarycentricMethod method = {});

    /*!
     * \brief Interpolate \p sourceValues at each point in \p targets.
     * \param sourceValues  Values at each source mesh vertex (span, length = nVertices).
     * \param targets       Query points in source mesh CRS.
     * \param outValues     Output values (size = targets.size()); NaN if outside mesh.
     */
    void interpolate(std::span<const double>                          sourceValues,
                     std::span<const HydroCouple::Spatial::Point>    targets,
                     std::span<double>                                outValues) const;

private:
    const HydroCouple::Spatial::TIN *m_src;
    BarycentricMethod                 m_method;
    // Octree built lazily on first interpolate() call
    mutable std::unique_ptr<HydroCouple::Spatial::Octree> m_index;
};

/*!
 * \brief Interpolates values from a RegularGrid2D raster to mesh vertices.
 * Applies bilinear sampling at each vertex's (x, y) coordinate.
 */
class HYDROCOUPLESDK_EXPORT GridToMeshInterpolator {
public:
    explicit GridToMeshInterpolator(
        const HydroCouple::Spatial::RegularGrid2D *sourceGrid,
        BilinearMethod method = {});

    void interpolate(std::span<const double>                       gridValues,
                     std::span<const HydroCouple::Spatial::Point> meshVertices,
                     std::span<double>                             outValues) const;

private:
    const HydroCouple::Spatial::RegularGrid2D *m_src;
    BilinearMethod                              m_method;
};

/*!
 * \brief Interpolates mesh vertex values onto a regular grid.
 * Uses IDW (default) or barycentric depending on method.
 */
class HYDROCOUPLESDK_EXPORT MeshToGridInterpolator {
public:
    explicit MeshToGridInterpolator(
        const HydroCouple::Spatial::TIN        *sourceMesh,
        const HydroCouple::Spatial::RegularGrid2D *targetGrid,
        IDWMethod method = {});

    void interpolate(std::span<const double> meshValues,
                     std::span<double>       gridValues) const;

private:
    const HydroCouple::Spatial::TIN          *m_mesh;
    const HydroCouple::Spatial::RegularGrid2D *m_grid;
    IDWMethod                                   m_method;
};

} // namespace HydroCouple::Tools::Interpolation
```

### 17.12 `HydroCoupleTools` Library Target in CMakeLists

```cmake
option(BUILD_TOOLS "Build HydroCouple Tools library" ON)

if(BUILD_TOOLS)
    add_library(HydroCoupleTools SHARED)
    add_subdirectory(src/tools)

    target_compile_definitions(HydroCoupleTools PRIVATE HYDROCOUPLETOOLS_LIBRARY)
    target_compile_features(HydroCoupleTools PUBLIC cxx_std_20)

    target_include_directories(HydroCoupleTools PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

    target_link_libraries(HydroCoupleTools
        PUBLIC  HydroCoupleSDK GDAL::GDAL
        PRIVATE tinyxml2::tinyxml2)

    # Vendor Triangle for Delaunay generation
    add_library(triangle_lib STATIC
        src/vendor/triangle/triangle.c)
    target_compile_definitions(triangle_lib PRIVATE
        TRILIBRARY ANSI_DECLARATORS NO_TIMER)
    target_link_libraries(HydroCoupleTools PRIVATE triangle_lib)

    install(TARGETS HydroCoupleTools EXPORT HydroCoupleSDKTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY include/tools/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tools
        FILES_MATCHING PATTERN "*.h")
endif()
```

---

## 18. UGRID Conventions for Mesh Persistence (NetCDF and HDF5)

UGRID (CF-1.8 + UGRID 1.0) defines standard names, dimensions, variables, and
attributes for storing unstructured grids in NetCDF-4/HDF5.  All mesh write/read
in `HydroCouple::IO` follows UGRID 1.0 exactly so files are interoperable with
Delft3D, ADCIRC, FVCOM, and other UGRID-aware tools.

### 18.1 UGRID Topology Mapping

| UGRID concept | SDK class | UGRID variable name |
|---|---|---|
| Mesh topology | `TIN` / `PolyhedralSurface` | `mesh` (scalar integer, `cf_role = mesh_topology`) |
| Nodes | `VertexPool.x`, `VertexPool.y`, `VertexPool.z` | `node_x`, `node_y`, `node_z` |
| Face-node connectivity | `FacePool.firstEdge` + orbit traversal | `face_nodes` (2D: `[nFaces, maxNodesPerFace]`) |
| Edge-node connectivity | `QuadEdgePool.m_vertex` | `edge_nodes` (2D: `[nEdges, 2]`) |
| Face-edge connectivity | derived from topology | `face_edges` (optional) |
| Face-face connectivity | derived from topology | `face_faces` (optional) |
| Data on nodes | `TINComponentDataItem` (Node type) | `node_data_var` |
| Data on edges | `TINComponentDataItem` (Edge type) | `edge_data_var` |
| Data on faces | `TINComponentDataItem` (Cell type) | `face_data_var` |

### 18.2 `UGRIDMeshWriter` (`include/io/netcdf/ugridmeshwriter.h`)

```cpp
namespace HydroCouple::IO::NetCDF {

/*!
 * \brief Writes SDK mesh objects to NetCDF-4 following the UGRID 1.0 convention.
 *
 * Reference: https://ugrid-conventions.github.io/ugrid-conventions/
 *
 * Creates or appends to a NetCDF-4 file.  One mesh per group (or root for
 * single-mesh files). Multiple meshes (e.g., 2D surface + 1D network) are
 * each written to a named sub-group.
 */
class HYDROCOUPLESDK_EXPORT UGRIDMeshWriter {
public:
    /*!
     * \brief Open a NetCDF-4 file for writing.
     * \param filePath  Path to the .nc file.
     * \param overwrite If false and file exists, appends (creates a new group).
     */
    explicit UGRIDMeshWriter(std::string_view filePath, bool overwrite = true);
    ~UGRIDMeshWriter();

    /*!
     * \brief Write a TIN / PolyhedralSurface to the file.
     *
     * Produces the following UGRID variables (CF-1.8 names):
     *   - mesh          (scalar int, mesh_topology, face_dimension, node_dimension, edge_dimension)
     *   - node_x        (double [nNodes])
     *   - node_y        (double [nNodes])
     *   - node_z        (double [nNodes])  — if hasZ
     *   - face_nodes    (int    [nFaces, maxNodesPerFace])  fill_value = -1
     *   - edge_nodes    (int    [nEdges, 2])
     *
     * \param mesh       The mesh to write.
     * \param meshName   Group name (default "mesh").
     * \param crsWkt     Coordinate reference system WKT (stored as global attribute).
     */
    bool writeMesh(const HydroCouple::Spatial::PolyhedralSurface *mesh,
                   std::string_view meshName = "mesh",
                   std::string_view crsWkt   = "");

    /*!
     * \brief Write data variables associated with a previously written mesh.
     *
     * \param meshName   Must match a name used in writeMesh().
     * \param varName    UGRID data variable name.
     * \param location   "node", "edge", or "face".
     * \param values     Values (length = nNodes | nEdges | nFaces).
     * \param units      CF units string (e.g., "m s-1").
     * \param longName   Human-readable description.
     * \param fillValue  Missing-data sentinel.
     */
    bool writeDataVariable(std::string_view meshName,
                           std::string_view varName,
                           std::string_view location,
                           std::span<const double> values,
                           std::string_view units     = "",
                           std::string_view longName  = "",
                           double           fillValue = 9.96921e+36);

    /*!
     * \brief Write a time series of data variables.
     * \param times    Julian days (length = nTimesteps).
     * \param values   Flat [nTimesteps × nSpatial] row-major array.
     */
    bool writeTimeSeriesData(std::string_view meshName,
                             std::string_view varName,
                             std::string_view location,
                             std::span<const double> times,
                             std::span<const double> values,
                             int                     nSpatial,
                             std::string_view units    = "",
                             std::string_view longName = "");

    std::string errorMsg() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace HydroCouple::IO::NetCDF
```

### 18.3 `UGRIDMeshReader` (`include/io/netcdf/ugridmeshreader.h`)

```cpp
namespace HydroCouple::IO::NetCDF {

/*!
 * \brief Reads UGRID 1.0 compliant NetCDF-4 files and reconstructs
 *        SDK mesh objects with their quad-edge topology.
 *
 * Discovers mesh topology variables by scanning for variables with
 * cf_role = "mesh_topology" in the root group and all sub-groups.
 */
class HYDROCOUPLESDK_EXPORT UGRIDMeshReader {
public:
    explicit UGRIDMeshReader(std::string_view filePath);
    ~UGRIDMeshReader();

    /*!
     * \brief List all mesh names found in the file.
     */
    [[nodiscard]] std::vector<std::string> meshNames() const;

    /*!
     * \brief Read a mesh topology and reconstruct a TIN or PolyhedralSurface.
     *
     * Reconstructs the quad-edge topology from face_nodes and edge_nodes arrays.
     * A TIN is returned if all faces have exactly 3 nodes;
     * a PolyhedralSurface is returned otherwise.
     *
     * \param meshName  Name from meshNames(). Default = first mesh found.
     * \param outCrsWkt Filled with the CRS WKT from the file attributes.
     */
    [[nodiscard]] std::unique_ptr<HydroCouple::Spatial::PolyhedralSurface>
    readMesh(std::string_view meshName = "",
             std::string     *outCrsWkt = nullptr) const;

    /*!
     * \brief Read a data variable from a given mesh.
     * \param meshName  Target mesh.
     * \param varName   Variable name as stored in the file.
     * \param location  "node", "edge", or "face".
     * \return          Values in mesh index order; empty on failure.
     */
    [[nodiscard]] std::vector<double>
    readDataVariable(std::string_view meshName,
                     std::string_view varName,
                     std::string_view location) const;

    /*!
     * \brief Read a time-indexed data variable.
     * \param times  Filled with Julian Day timestamps.
     * \param values Filled flat [nTimes × nSpatial] row-major.
     */
    bool readTimeSeriesData(std::string_view meshName,
                            std::string_view varName,
                            std::string_view location,
                            std::vector<double> &times,
                            std::vector<double> &values,
                            int                 &nSpatial) const;

    std::string errorMsg() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace HydroCouple::IO::NetCDF
```

### 18.4 `UGRIDConventions` — UGRID Attribute Constants

```cpp
namespace HydroCouple::IO::NetCDF::UGRID {

// Standard CF/UGRID global attributes
inline constexpr std::string_view CONVENTIONS     = "CF-1.8 UGRID-1.0";
inline constexpr std::string_view MESH_TOPOLOGY   = "mesh_topology";
inline constexpr std::string_view FACE_DIMENSION  = "face_dimension";
inline constexpr std::string_view NODE_DIMENSION  = "node_dimension";
inline constexpr std::string_view EDGE_DIMENSION  = "edge_dimension";

// Standard variable names
inline constexpr std::string_view NODE_X          = "node_x";
inline constexpr std::string_view NODE_Y          = "node_y";
inline constexpr std::string_view NODE_Z          = "node_z";
inline constexpr std::string_view FACE_NODES      = "face_nodes";
inline constexpr std::string_view EDGE_NODES      = "edge_nodes";
inline constexpr std::string_view FACE_EDGES      = "face_edges";
inline constexpr std::string_view FACE_FACES      = "face_faces";

// Location strings for data variables
inline constexpr std::string_view LOC_NODE        = "node";
inline constexpr std::string_view LOC_EDGE        = "edge";
inline constexpr std::string_view LOC_FACE        = "face";

// Fill value for connectivity arrays (UGRID spec §3.4)
inline constexpr int FILL_VALUE_INT               = -1;

} // namespace HydroCouple::IO::NetCDF::UGRID
```

### 18.5 UGRID in HDF5 (`include/io/hdf5/ugridmeshh5writer.h`)

```cpp
namespace HydroCouple::IO::HDF5 {

/*!
 * \brief Writes UGRID-convention mesh data to HDF5.
 *
 * Layout mirrors the NetCDF-4/UGRID layout exactly (HDF5 is the binary
 * backend of NetCDF-4, so variable names and attribute names are identical):
 *
 *   /mesh/                  — root group per mesh
 *     ├─ node_x             — dataset [nNodes]
 *     ├─ node_y             — dataset [nNodes]
 *     ├─ node_z             — dataset [nNodes]  (if hasZ)
 *     ├─ face_nodes         — dataset [nFaces, maxNodesPerFace]
 *     ├─ edge_nodes         — dataset [nEdges, 2]
 *     ├─ .attrs/
 *     │    cf_role          = "mesh_topology"
 *     │    face_dimension   = "nFaces"
 *     │    node_dimension   = "nNodes"
 *     │    Conventions      = "CF-1.8 UGRID-1.0"
 *     │    crs_wkt          = "<wkt string>"
 *     └─ data/
 *          ├─ <varName>_node  — node data
 *          ├─ <varName>_edge  — edge data
 *          └─ <varName>_face  — face data
 */
class HYDROCOUPLESDK_EXPORT UGRIDMeshH5Writer {
public:
    explicit UGRIDMeshH5Writer(std::string_view filePath, bool overwrite = true);
    ~UGRIDMeshH5Writer();

    bool writeMesh(const HydroCouple::Spatial::PolyhedralSurface *mesh,
                   std::string_view meshName = "mesh",
                   std::string_view crsWkt   = "");

    bool writeDataVariable(std::string_view meshName,
                           std::string_view varName,
                           std::string_view location,
                           std::span<const double> values,
                           std::string_view units    = "",
                           std::string_view longName = "");

    bool writeTimeSeriesData(std::string_view meshName,
                             std::string_view varName,
                             std::string_view location,
                             std::span<const double> times,
                             std::span<const double> values,
                             int nSpatial);

    std::string errorMsg() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace HydroCouple::IO::HDF5
```

### 18.6 UGRID Reconstruction — Quad-Edge Rebuild from Face-Node Table

When reading `face_nodes`, the topology must be reconstructed into the quad-edge
`PolyhedralSurface`. The algorithm:

1. Create all `Vertex` handles from `node_x`, `node_y`, `node_z`.
2. For each face row `[v0, v1, v2, ..., vN]` in `face_nodes`:
   a. Create a `Polygon` handle in `FacePool`.
   b. For each consecutive pair `(vi, v_{i+1 mod N})`:
      - If edge `vi → v_{i+1}` already exists (from a previous face): reuse it, set `right` face.
      - Else: call `createFaceEdge(face, vi, v_{i+1})` which calls `QuadEdgePool::allocateQuad()` + `splice()`.
3. After all faces: link boundary edges (edges with `right == null`).

This algorithm is O(E) in the number of edges, and correctly reconstructs the full
quad-edge topology from the compact face-node connectivity table.

---

## 19. Updated vcpkg.json (Tools module dependencies)

```json
{
  "name": "hydrocouplesdk",
  "version-semver": "1.0.0",
  "dependencies": [
    { "name": "vcpkg-cmake",        "host": true },
    { "name": "vcpkg-cmake-config", "host": true },
    "gdal",
    "netcdf-cxx4",
    "hdf5",
    "tinyxml2",
    { "name": "gtest", "default-features": false },
    { "name": "openmp", "platform": "!uwp" }
  ],
  "features": {
    "mpi": {
      "description": "Enable MPI support",
      "dependencies": ["mpi"]
    },
    "tools": {
      "description": "Mesh generation and interpolation tools",
      "dependencies": ["gdal"]
    }
  }
}
```

---

## 21. License Change — LGPL-3.0 → MIT

### 21.1 Rationale

The HydroCouple interface project already uses **MIT**. Aligning the SDK to MIT:

- Removes the LGPL copyleft requirement that consumers must re-link against newer SDK versions
- Matches the dependency licenses (nlohmann/json: MIT; tinyxml2: zlib; Triangle: custom non-commercial — **see §21.5**)
- Simplifies downstream adoption in both open-source and commercial modelling software

### 21.2 New `LICENSE` File

Replace the existing `License.md` with a standard `LICENSE` file at the repository root:

```
MIT License

Copyright (c) 2014-2026 Caleb Buahin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

### 21.3 Files to Update

Every `.h` and `.cpp` file currently carries the LGPL header block:

```cpp
// This file and its associated files, and libraries are free software.
// You can redistribute it and/or modify it under the terms of the
// Lesser GNU Lesser General Public License ...
```

Replace with the standard MIT short-form header:

```cpp
// SPDX-License-Identifier: MIT
// Copyright (c) 2014-2026 Caleb Buahin
```

This applies to **all** files under `include/`, `src/`, `tests/`, and `docs/`.

### 21.4 CMakeLists.txt and vcpkg.json

**`CMakeLists.txt`** — update the project declaration:
```cmake
project(
    HydroCoupleSDK
    VERSION 1.0.0
    LANGUAGES C CXX
    HOMEPAGE_URL "https://hydrocouple.org"
    DESCRIPTION  "HydroCouple C++ Modeling Framework SDK"
)
# Add after project():
set(PROJECT_LICENSE "MIT")
```

Update CPack metadata:
```cmake
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_NUGET_PACKAGE_LICENSE_URL "https://opensource.org/licenses/MIT")
```

**`vcpkg.json`** — change the license field:
```json
"license": "MIT"
```

**`install()` rule** — install the new `LICENSE` file:
```cmake
install(FILES LICENSE README.md DESTINATION .)
```

### 21.5 Triangle Library — License Caveat

Shewchuk's Triangle library (`src/vendor/triangle/triangle.c`) carries a
**non-commercial restriction**:

> "These programs may be freely redistributed under the condition that the
> copyright notices are not removed, and no compensation is received. Private,
> research, and institutional use is free. Distribution of this code as part
> of a commercial product is permissible **only if the use within the commercial
> product is itself non-commercial**."

This is **incompatible with commercial use** of the `DelaunayMeshGenerator`.
Two options — choose one before shipping:

| Option | Description |
|---|---|
| **A — Restrict to non-commercial builds** | Gate `DelaunayMeshGenerator` behind `USE_TRIANGLE=ON` (default OFF for releases); document the restriction clearly. SDK core remains MIT. |
| **B — Replace Triangle** | Substitute with a fully permissive Delaunay library: [`CDT`](https://github.com/artem-ogre/CDT) (MIT), or [`Fade2D`](https://www.geom.at/fade2d/) (commercial licence required separately). CDT is available in vcpkg as `cdt`. |

**Recommended:** Option B using `CDT` (MIT, header-only, constrained Delaunay, vcpkg `cdt`).
Add to `vcpkg.json` features:
```json
"tools": {
    "description": "Mesh generation and interpolation tools",
    "dependencies": ["gdal", "cdt"]
}
```

### 21.6 Dependency License Compatibility Summary

| Dependency | License | Compatible with MIT SDK? |
|---|---|---|
| GDAL | MIT / X11 | Yes |
| NetCDF-C / netCDF-CXX4 | MIT-like (UCAR/Unidata) | Yes |
| HDF5 | BSD-like (HDF Group) | Yes |
| nlohmann/json | MIT | Yes |
| Google Test | BSD 3-Clause | Yes (tests only) |
| OpenMP runtime | Various (GCC: GPL runtime exception; LLVM: MIT) | Yes |
| MPI | Varies (OpenMPI: BSD; MPICH: MIT-like) | Yes |
| Triangle (Shewchuk) | Custom non-commercial | **No — see §21.5** |
| CDT (replacement) | MIT | Yes |
| tinyxml2 (if retained) | zlib | Yes |

---

## 22. MPI Communication — Components and Proxies

### 22.1 Current State

`AbstractModelComponent` already:
- Creates a component-specific MPI sub-communicator via `MPI_Comm_create_group` from `MPI_COMM_WORLD`
- Stores `m_ComponentMPIComm`, `m_ComponentMPIGroup`, `m_worldGroup`
- Exposes `mpiCommunicator()` returning `MPI_Comm`
- Tracks `m_mpiAllocatedProcesses` (the set of world ranks assigned to this component)

**Gaps:**
- `mpiCommunicator()` exists on `AbstractModelComponent` but is **absent from `IModelComponent`**
- `IProxyModelComponent` has only 3 read-only accessors — no connection management, no data transfer protocol
- No `ProxyModelComponent` concrete implementation exists in the SDK
- No worker-rank event loop exists
- No message protocol (tags, envelope, serialisation) is defined
- Status changes on worker ranks are never propagated back to the proxy on the main rank
- `IOutput::updateValues()` has no mechanism to transfer data across ranks

### 22.2 Architecture — Command-Worker over MPI Inter-Communicators

```
┌──────────────────────────────────────────────────────────────────────┐
│  Rank 0  (orchestration / "main" rank)                               │
│                                                                      │
│  WorkflowComponent                                                   │
│   ├── ProxyModelComponent A  ──intercomm──► worker group {1,2}      │
│   └── ProxyModelComponent B  ──intercomm──► worker group {3}        │
│                                                                      │
│  MPI_Iprobe loop (std::jthread)                                      │
│   └── receives StatusChanged messages from all worker groups         │
└──────────────────────────────────────────────────────────────────────┘
         │  MPI_COMM_WORLD                          │
┌────────▼──────────────────────┐  ┌───────────────▼──────────────────┐
│  Ranks 1–2  (worker group A)  │  │  Rank 3  (worker group B)         │
│                               │  │                                   │
│  ComponentWorker              │  │  ComponentWorker                  │
│   └── ModelComponent A (real) │  │   └── ModelComponent B (real)     │
│        event loop             │  │        event loop                 │
└───────────────────────────────┘  └───────────────────────────────────┘
```

**Key design decisions:**

1. **One MPI_Comm per component group** — `MPI_Comm_create_group` on `MPI_COMM_WORLD` creates `m_ComponentMPIComm` for each component's allocated process set (already done).

2. **MPI_Intercomm_create** bridges the main rank's singleton group to each component's worker group. This gives the proxy on rank 0 a dedicated channel to each component with no tag collision risk across components.

3. **Worker event loop** — each non-main rank runs `ComponentWorker::run()` (a blocking receive loop) until it receives a `Shutdown` envelope. This loop is started by `main()` before the workflow begins.

4. **Non-blocking updates** — `updateValues()` posts `MPI_Isend` on the proxy; the worker posts `MPI_Irecv`, computes, then sends results back. The proxy calls `MPI_Wait` only when a downstream consumer actually requests the values.

5. **Status propagation** — worker rank fires `MPI_Isend(StatusChanged)` on every `setStatus()`. The main rank's background `std::jthread` (`MPI_Iprobe` + drain loop) wakes up, updates the proxy's status, and fires the proxy's `componentStatusChanged` signal.

### 22.3 MPI Message Protocol

#### Envelope (fixed 32 bytes, always sent first)

```cpp
namespace HydroCouple::SDK::MPI {

enum class Op : int32_t {
    // ── Lifecycle ──────────────────────────────────────────────────────
    Initialize     = 0x01,
    Validate       = 0x02,
    Prepare        = 0x03,
    Update         = 0x04,
    Finish         = 0x05,
    // ── Data exchange ─────────────────────────────────────────────────
    UpdateValues   = 0x10,   ///< proxy → worker: compute and send back output values
    GetValues      = 0x11,   ///< proxy → worker: fetch cached output values
    SetArgument    = 0x12,   ///< proxy → worker: push argument value
    GetArgument    = 0x13,   ///< worker → proxy: pull argument value
    // ── Status ────────────────────────────────────────────────────────
    StatusQuery    = 0x20,   ///< proxy → worker: what is your current status?
    StatusChanged  = 0x21,   ///< worker → proxy: async status notification
    // ── Control ───────────────────────────────────────────────────────
    Shutdown       = 0xFF,   ///< proxy → worker: terminate event loop
};

struct alignas(8) Envelope {
    int32_t  op;             ///< Op enum value
    int32_t  seqNo;          ///< monotonic sequence number (detect reordering)
    int32_t  sourceRank;     ///< world rank of sender
    int32_t  replyTag;       ///< MPI tag to use for the reply message
    int32_t  itemIndex;      ///< output/input/argument slot index
    int32_t  payloadBytes;   ///< byte length of the payload message that follows (0 = none)
    int64_t  timeIndex;      ///< time step index (-1 = not applicable)
};
static_assert(sizeof(Envelope) == 32);

} // namespace HydroCouple::SDK::MPI
```

All envelopes use **tag `0`** on the inter-communicator
(`MPI_TAG_UB` is at least 32767; reserving tag 0 for envelopes avoids collision).
Reply messages use `replyTag` from the envelope so the proxy can match responses to requests.

#### Reply format

Replies carry a fixed-size status code followed by optional payload:

```cpp
struct alignas(4) Reply {
    int32_t  op;           ///< echoes the request Op
    int32_t  seqNo;        ///< echoes the request seqNo
    int32_t  status;       ///< 0 = success; non-zero = error code
    int32_t  payloadBytes; ///< byte length of the optional payload that follows
};
static_assert(sizeof(Reply) == 16);
```

### 22.4 Value Serialisation — `MPIValuePacker`

```cpp
namespace HydroCouple::SDK::MPI {

/*!
 * \brief Serialises and deserialises hydrocouple_variant arrays for MPI transfer.
 *
 * Fast path: when all values are double (the common case for numeric data),
 * packs directly into a double[] buffer with zero overhead.
 * General path: uses MPI_Pack/MPI_Unpack with type tags for mixed-type arrays.
 */
class HYDROCOUPLESDK_EXPORT MPIValuePacker {
public:
    /*!
     * \brief Pack values into a flat byte buffer suitable for MPI_Send.
     * \param values  Source values (span, length = count).
     * \param comm    Communicator (needed by MPI_Pack for size calculation).
     * \param buf     Output byte buffer; resized as needed.
     */
    static void pack(std::span<const HydroCouple::hydrocouple_variant> values,
                     MPI_Comm comm, std::vector<char> &buf);

    /*!
     * \brief Unpack a byte buffer received from MPI_Recv back into variants.
     * \param buf     Received byte buffer.
     * \param comm    Communicator.
     * \param values  Output values; must be pre-sized to the expected count.
     */
    static void unpack(std::span<const char> buf, MPI_Comm comm,
                       std::span<HydroCouple::hydrocouple_variant> values);

    /*!
     * \brief Fast path: pack a contiguous double array (no type tags).
     * Use when the caller already knows all values are double.
     */
    static void packDoubles(std::span<const double> values,
                            std::vector<char> &buf);

    static void unpackDoubles(std::span<const char> buf,
                              std::span<double> values);

    /*! \brief Return the MPI_Datatype for a variant value. */
    static MPI_Datatype datatypeFor(const HydroCouple::hydrocouple_variant &v);
};

} // namespace HydroCouple::SDK::MPI
```

### 22.5 `ComponentWorker` — Worker-Rank Event Loop

```cpp
namespace HydroCouple::SDK {

/*!
 * \brief Runs on every non-main rank. Owns the real ModelComponent and
 *        processes incoming MPI envelopes until Shutdown is received.
 *
 * Usage on each worker rank in main():
 * \code
 *     int worldRank;
 *     MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
 *     if (worldRank != 0) {
 *         ComponentWorker worker;
 *         worker.run();   // blocks until Shutdown
 *         MPI_Finalize();
 *         return 0;
 *     }
 *     // rank 0 continues to run the WorkflowComponent
 * \endcode
 */
class HYDROCOUPLESDK_EXPORT ComponentWorker {
public:
    /*!
     * \brief Register the real component this worker owns.
     * Called before run() on the worker rank.
     */
    void setComponent(HydroCouple::IModelComponent *component);

    /*!
     * \brief Block on MPI_Recv, dispatching envelopes until Op::Shutdown.
     * \param stopToken C++20 stop token for external cancellation.
     */
    void run(std::stop_token stopToken = {});

    /*!
     * \brief Non-blocking poll — process one pending envelope if available.
     * Useful when the worker rank needs to interleave its own compute work.
     * Returns true if an envelope was processed.
     */
    bool poll();

private:
    void dispatch(const HydroCouple::SDK::MPI::Envelope &env,
                  MPI_Comm interComm, int srcRank);

    // ── Dispatch handlers ─────────────────────────────────────────────────
    void handleInitialize (const MPI::Envelope &, MPI_Comm, int srcRank);
    void handleValidate   (const MPI::Envelope &, MPI_Comm, int srcRank);
    void handlePrepare    (const MPI::Envelope &, MPI_Comm, int srcRank);
    void handleUpdate     (const MPI::Envelope &, MPI_Comm, int srcRank);
    void handleFinish     (const MPI::Envelope &, MPI_Comm, int srcRank);
    void handleUpdateValues(const MPI::Envelope &, MPI_Comm, int srcRank);
    void handleGetValues  (const MPI::Envelope &, MPI_Comm, int srcRank);
    void handleSetArgument(const MPI::Envelope &, MPI_Comm, int srcRank);
    void handleStatusQuery(const MPI::Envelope &, MPI_Comm, int srcRank);

    // ── Reply helpers ─────────────────────────────────────────────────────
    void sendReply(MPI_Comm comm, int destRank, int replyTag,
                   const MPI::Reply &header,
                   std::span<const char> payload = {});

    HydroCouple::IModelComponent *m_component = nullptr;
    int32_t m_seqCounter = 0;
};

} // namespace HydroCouple::SDK
```

### 22.6 `ProxyModelComponent` — Main-Rank Proxy

```cpp
namespace HydroCouple::SDK {

/*!
 * \brief Stands in for a remote IModelComponent on the main (rank-0) process.
 *
 * Every IModelComponent lifecycle call (initialize, validate, prepare, update,
 * finish) is serialised into an MPI envelope and sent over the inter-communicator
 * to the worker rank(s). Responses are awaited synchronously (default) or
 * asynchronously (via the non-blocking update path for updateValues).
 *
 * Status changes on the worker rank arrive via a background std::jthread that
 * polls MPI_Iprobe and fires the proxy's componentStatusChanged signal on the
 * main thread.
 */
class HYDROCOUPLESDK_EXPORT ProxyModelComponent
    : public HydroCouple::Identity,
      public virtual HydroCouple::IProxyModelComponent {
public:
    /*!
     * \brief Construct a proxy for the component with the given id.
     * \param id            Unique component id (must match the real component).
     * \param parentRank    World rank of the primary worker rank.
     * \param workerGroup   All world ranks allocated to this component.
     */
    ProxyModelComponent(std::string_view id,
                        int parentRank,
                        const std::vector<int> &workerGroup);

    ~ProxyModelComponent() override;

    // ── IProxyModelComponent ──────────────────────────────────────────────
    int         parentMpiProcessRank()  const override;
    std::string parentProcessAddress()  const override;
    std::string parentId()              const override;

    // ── Connection management ─────────────────────────────────────────────
    /*!
     * \brief Create the MPI inter-communicator to the worker group.
     * Must be called collectively on rank 0 and on all worker ranks
     * simultaneously (MPI collective operation).
     */
    bool mpiConnect(std::string &errorMessage);

    [[nodiscard]] bool isMpiConnected() const noexcept;

    /*! \brief The inter-communicator to this component's worker group. */
    [[nodiscard]] MPI_Comm mpiInterCommunicator() const noexcept;

    // ── IModelComponent lifecycle (forwarded to worker via MPI) ───────────
    IModelComponent::ComponentStatus status() const override;
    std::vector<HydroCouple::IArgument*> arguments() const override;
    std::vector<HydroCouple::IInput*>    inputs()    const override;
    std::vector<HydroCouple::IOutput*>   outputs()   const override;

    void initialize() override;
    std::vector<std::string> validate() override;
    void prepare()  override;
    void update(const std::initializer_list<HydroCouple::IOutput*> &required = {}) override;
    void finish()   override;

    // ── IModelComponent MPI methods ───────────────────────────────────────
    int              mpiNumOfProcesses()  const override;
    int              mpiProcessRank()     const override;
    void             mpiSetProcessRank(int r) override;
    std::set<int>    mpiAllocatedProcesses() const override;
    void             mpiAllocateProcesses(const std::set<int> &procs) override;
    void             mpiClearAllocatedProcesses() override;

    // ── Other IModelComponent stubs (no-op on the proxy) ─────────────────
    HydroCouple::IModelComponentInfo *componentInfo() const override;
    std::string       referenceDirectory()        const override;
    void              setReferenceDirectory(std::string_view) override;
    bool              hasEditor() const override { return false; }
    void              showEditor(void*) override {}
    bool              hasViewer() const override { return false; }
    void              showViewer(void*) override {}
    const HydroCouple::IWorkflowComponent *workflow() const override;
    void              setWorkflow(const HydroCouple::IWorkflowComponent*) override;
    int               index() const override;
    void              setIndex(int) override;
    std::vector<HydroCouple::IComponentDataItem*> results() const override;

private:
    // ── MPI helpers ───────────────────────────────────────────────────────
    void sendEnvelope(MPI::Op op, int itemIndex = -1,
                      int64_t timeIndex = -1,
                      std::span<const char> payload = {});

    MPI::Reply awaitReply(int replyTag, std::vector<char> *payloadOut = nullptr);

    int32_t nextSeq() noexcept { return m_seqCounter.fetch_add(1); }

    // ── Status polling thread ─────────────────────────────────────────────
    void startStatusThread();
    void stopStatusThread();
    void pollStatusMessages();   ///< called by the background jthread

    // ── Cached state (kept in sync with the real component) ───────────────
    mutable HydroCouple::IModelComponent::ComponentStatus m_status
        = HydroCouple::IModelComponent::ComponentStatus::Created;
    mutable std::vector<std::string>    m_lastValidationMessages;

    // ── Connection ────────────────────────────────────────────────────────
    int             m_parentRank;
    std::vector<int> m_workerGroup;
    MPI_Comm        m_interComm   = MPI_COMM_NULL;
    bool            m_connected   = false;

    std::atomic<int32_t>  m_seqCounter{0};
    std::jthread          m_statusThread;   ///< background status poller
    std::atomic<bool>     m_running{false};
};

} // namespace HydroCouple::SDK
```

### 22.7 `ProxyOutput` — Data Transfer for `IOutput`

On the main rank, each output of the real component is represented by a `ProxyOutput`.
When a downstream consumer calls `updateValues()` on the proxy output, it:

1. Sends `Op::UpdateValues` envelope to the worker rank
2. Worker calls `realOutput->updateValues(nullptr)` then packs the result values
3. Worker sends back `Reply` + `double[]` payload
4. `ProxyOutput` stores the payload in a local `std::vector<double>` cache
5. Subsequent `getValue()` / `getValues()` calls on the proxy read from the cache

```cpp
namespace HydroCouple::SDK {

class HYDROCOUPLESDK_EXPORT ProxyOutput
    : public HydroCouple::AbstractOutput {
public:
    ProxyOutput(std::string_view id,
                int outputIndex,
                ProxyModelComponent *proxy,
                HydroCouple::ValueDefinition *valueDef,
                const std::vector<HydroCouple::IDimension*> &dims);

    void updateValues(const HydroCouple::IInput *querySpecifier) override;

    void getValue(HydroCouple::hydrocouple_variant &data,
                  const std::initializer_list<int> &dims) const override;
    void getValues(HydroCouple::hydrocouple_variant *data,
                   const std::initializer_list<int> &dims,
                   const std::initializer_list<int> &lengths = {}) const override;

private:
    void fetchFromWorker() const;  ///< called lazily inside getValue/getValues

    int                      m_outputIndex;
    ProxyModelComponent     *m_proxy;
    mutable std::vector<double> m_cache;  ///< last received values
    mutable bool             m_dirty = true; ///< true = need to re-fetch
};

} // namespace HydroCouple::SDK
```

### 22.8 Required Interface Additions (`HydroCouple` project)

The following are the **only changes needed in the interface definitions project**.
They resolve genuine gaps that became apparent from the SDK implementation.

#### 22.8.1 Add `mpiCommunicator()` to `IModelComponent`

`AbstractModelComponent` already has this method but it is absent from the interface,
making it impossible to call through the `IModelComponent*` pointer.

```cpp
// In hydrocouple.h — IModelComponent class
/*!
 * \brief Returns the MPI communicator for this component's allocated process group.
 * Returns MPI_COMM_SELF if only one rank is allocated, MPI_COMM_NULL if MPI is
 * not initialised. The return type is int for portability (MPI_Comm is int on
 * all standard MPI implementations).
 */
virtual int mpiCommunicatorHandle() const = 0;
```

#### 22.8.2 Expand `IProxyModelComponent`

```cpp
// In hydrocouple.h — IProxyModelComponent class
/*!
 * \brief Establish the MPI inter-communicator to the real component's worker group.
 * This is a collective MPI operation: must be called simultaneously on rank 0
 * and on all ranks in the worker group.
 * \param errorMessage  Filled on failure.
 * \return true on success.
 */
virtual bool mpiConnect(string &errorMessage) = 0;

/*!
 * \brief Returns true if the inter-communicator has been established.
 */
virtual bool isMpiConnected() const = 0;

/*!
 * \brief Returns the MPI inter-communicator handle to the worker group.
 * Returns MPI_COMM_NULL (0) before mpiConnect() is called.
 */
virtual int mpiInterCommunicatorHandle() const = 0;
```

#### 22.8.3 New `hydrocouplempi.h` (optional, recommended)

A new interface header in the HydroCouple project documenting the worker-side contract:

```cpp
// hydrocouplempi.h
namespace HydroCouple {

/*!
 * \brief Interface for the worker-rank event loop that owns a real IModelComponent.
 *
 * Implement this on each non-main MPI rank to receive and dispatch commands
 * from the main rank's ProxyModelComponent.
 */
class IMPIWorker {
public:
    virtual ~IMPIWorker() = default;

    /*! \brief Register the component this worker will service. */
    virtual void setComponent(IModelComponent *component) = 0;

    /*! \brief Block and process MPI envelopes until Shutdown is received. */
    virtual void run(/* std::stop_token */) = 0;

    /*! \brief Non-blocking: process one pending envelope if available. */
    virtual bool poll() = 0;
};

} // namespace HydroCouple
```

### 22.9 Lifecycle Sequence — Complete Walk-Through

```
Rank 0 (proxy)                           Rank 1 (worker)
──────────────────────────────────────────────────────────
main() sets up MPI_COMM_WORLD
proxy.mpiConnect()  ◄──────────────────── worker.mpiConnect()  [collective]
                    ◄──────────────────── ComponentWorker::run() starts
                                          [blocking on MPI_Recv]

proxy.initialize()
  sends Envelope{Initialize}  ──────────►
                               dispatch → component.initialize()
                  ◄────────── Reply{ok}
  proxy status = Initialized

proxy.validate()
  sends Envelope{Validate}    ──────────►
                               dispatch → component.validate()
                  ◄────────── Reply{ok} + packed vector<string>
  proxy returns validation messages

proxy.prepare()
  sends Envelope{Prepare}     ──────────►
                               dispatch → component.prepare()
                  ◄────────── Reply{ok}

── time loop ─────────────────────────────────────────────

proxy.update({proxyOutput})
  sends Envelope{Update}      ──────────►
                               dispatch → component.update()
                               component calls output.updateValues()
                               output computes values
                  ◄────────── Reply{ok}

consumer calls proxyOutput.updateValues()
  sends Envelope{UpdateValues, itemIndex=0} ────────────►
                               packs output values to double[]
                  ◄────────── Reply{ok} + double[] payload
  proxyOutput.m_cache = received values

consumer calls proxyOutput.getValue(dims)
  → reads from m_cache  [no MPI call needed]

── status change on worker ───────────────────────────────

component.setStatus(Updated)
  sends Envelope{StatusChanged} (MPI_Isend) ──────────────►
  [background jthread on rank 0 MPI_Iprobe]
  ◄─────────────────── fires proxy.componentStatusChanged signal

── end ───────────────────────────────────────────────────

proxy.finish()
  sends Envelope{Finish}       ──────────►
                                dispatch → component.finish()
                   ◄────────── Reply{ok}

proxy destructor / workflow shutdown
  sends Envelope{Shutdown}     ──────────►
                                ComponentWorker::run() returns
  MPI_Finalize()                MPI_Finalize()
```

### 22.10 Argument Synchronisation

Before `initialize()` is called, the workflow must push any argument values from the
main rank to the worker rank. The proxy iterates its cached arguments and sends each
one as `Op::SetArgument` + JSON payload:

```cpp
void ProxyModelComponent::pushArguments() {
    for (int i = 0; i < (int)m_arguments.size(); ++i) {
        std::string json = m_arguments[i]->toString();  // IArgument::toString()
        std::vector<char> payload(json.begin(), json.end());
        sendEnvelope(MPI::Op::SetArgument, i, -1, payload);
        awaitReply(nextSeq());
    }
}
```

Worker's `handleSetArgument()` calls `m_component->arguments()[i]->initialize(json, ArgumentInputType::JSON, msg)`.

### 22.11 New SDK Files

| File | Description |
|---|---|
| `include/hydrocouple/mpi/mpienvelope.h` | `Envelope`, `Reply`, `Op` enum |
| `include/hydrocouple/mpi/mpivaluepacker.h` | `MPIValuePacker` — serialise `hydrocouple_variant[]` |
| `include/hydrocouple/mpi/componentworker.h` | `ComponentWorker` — worker-rank event loop |
| `include/hydrocouple/mpi/proxymodecomponent.h` | `ProxyModelComponent` |
| `include/hydrocouple/mpi/proxyoutput.h` | `ProxyOutput` |
| `include/hydrocouple/mpi/proxyinput.h` | `ProxyInput` (mirrors `ProxyOutput` for input direction) |
| `src/hydrocouple/mpi/componentworker.cpp` | |
| `src/hydrocouple/mpi/proxymodelcomponent.cpp` | |
| `src/hydrocouple/mpi/proxyoutput.cpp` | |
| `src/hydrocouple/mpi/mpivaluepacker.cpp` | |

All files are compiled only when `USE_MPI=ON`.

### 22.12 Changes Required in the Interface Project (`HydroCouple`)

| Change | File | Justification |
|---|---|---|
| Add `mpiCommunicatorHandle()` to `IModelComponent` | `hydrocouple.h` | Method exists in SDK but missing from interface — breaks polymorphic use |
| Add `mpiConnect()`, `isMpiConnected()`, `mpiInterCommunicatorHandle()` to `IProxyModelComponent` | `hydrocouple.h` | Current interface has no connection management at all |
| New `hydrocouplempi.h` with `IMPIWorker` | new file | Documents the worker-rank contract so third-party components can interoperate |

These are **additive only** — no existing methods are changed or removed.

## 20. Updated Implementation Order

| Phase | Description | Files |
|---|---|---|
| … (1–18 unchanged from §14) | … | … |
| 19 | C++20 upgrade: CMAKE_CXX_STANDARD 20; adopt span, concepts, ranges | All CMakeLists |
| 20 | `TerrainDomain` + `ConstraintSegment` + `RegionMarker` | 4 new files |
| 21 | `IMeshGenerator` concept + `MeshResult` (C++20) | 2 new files |
| 22 | `DelaunayMeshGenerator` (port from openswmm.gui, Qt removed) | 2 files |
| 23 | `RectilinearGrid2DGenerator` + `RectilinearGrid3DGenerator` | 4 files |
| 24 | `CurvilinearGrid2DGenerator` + `CurvilinearGrid3DGenerator` (TFI) | 4 files |
| 25 | `TerrainSampler` + `TerrainThinner` (port from openswmm.gui) | 4 files |
| 26 | Interpolation methods (bilinear, barycentric, IDW, nearest, natural) | 5 files |
| 27 | Spatial interpolators (mesh↔mesh, grid↔mesh) | 4 files |
| 28 | `UGRIDMeshWriter` + `UGRIDMeshReader` (NetCDF) | 4 files |
| 29 | `UGRIDMeshH5Writer` + `UGRIDMeshH5Reader` (HDF5) | 4 files |
| 30 | `UGRIDConventions` constants header | 1 file |
| 31 | GTest for all Tools + UGRID round-trip tests | ~12 new test files |
| 32 | **License change** — replace LICENSE file; update every file header to MIT SPDX; update CMakeLists + vcpkg.json; resolve Triangle §21.5 | All source files |
| 33 | **MPI interface additions** in HydroCouple project — `mpiCommunicatorHandle()`, expanded `IProxyModelComponent`, new `hydrocouplempi.h` | hydrocouple.h + 1 new file |
| 34 | `MPIEnvelope`, `MPIValuePacker`, `ComponentWorker` | 4 new files |
| 35 | `ProxyModelComponent`, `ProxyOutput`, `ProxyInput` | 6 new files |
| 36 | MPI GTest: lifecycle round-trip, UpdateValues transfer, StatusChanged propagation | ~4 new test files |

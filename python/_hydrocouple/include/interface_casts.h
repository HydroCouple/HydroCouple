/*
 * interface_casts.h
 *
 * dynamic_cast spellings for the bindings, one per side interface or
 * subtype a wrapper needs to recognise: Cython has no dynamic_cast of its
 * own (see differential_bridge.h, layered_casts.h). A null result means
 * "not that". The dispatchers use them to hand Python the most specific
 * wrapper for whatever C++ object they are given.
 */
#pragma once

#include "hydrocouple.h"
#include "hydrocouplespatial.h"
#include "hydrocoupletemporal.h"
#include "hydrocouplespatiotemporal.h"

namespace HydroCouple {
namespace Python {

// -- Core -------------------------------------------------------------------
inline IQuantity *asQuantity(IValueDefinition *value) { return dynamic_cast<IQuantity *>(value); }
inline IQuality *asQuality(IValueDefinition *value) { return dynamic_cast<IQuality *>(value); }
inline IMultiInput *asMultiInput(IInput *input) { return dynamic_cast<IMultiInput *>(input); }
inline IAdaptedOutput *asAdaptedOutput(IOutput *output) { return dynamic_cast<IAdaptedOutput *>(output); }
inline IDifferentiableAdaptedOutput *asDifferentiableAdapter(IAdaptedOutput *adapted)
{
    return dynamic_cast<IDifferentiableAdaptedOutput *>(adapted);
}
inline ICheckpointableAdaptedOutput *asCheckpointableAdapter(IAdaptedOutput *adapted)
{
    return dynamic_cast<ICheckpointableAdaptedOutput *>(adapted);
}
inline IArgument *asArgument(IComponentDataItem *item) { return dynamic_cast<IArgument *>(item); }
inline IInput *asInput(IComponentDataItem *item) { return dynamic_cast<IInput *>(item); }
inline IOutput *asOutput(IComponentDataItem *item) { return dynamic_cast<IOutput *>(item); }

// -- Temporal ---------------------------------------------------------------
inline Temporal::ITimeModelComponent *asTimeModelComponent(IModelComponent *component)
{
    return dynamic_cast<Temporal::ITimeModelComponent *>(component);
}

// -- Geometry ---------------------------------------------------------------
inline Spatial::IPoint *asPoint(Spatial::IGeometry *g) { return dynamic_cast<Spatial::IPoint *>(g); }
inline Spatial::IVertex *asVertex(Spatial::IGeometry *g) { return dynamic_cast<Spatial::IVertex *>(g); }
inline Spatial::ILineString *asLineString(Spatial::IGeometry *g) { return dynamic_cast<Spatial::ILineString *>(g); }
inline Spatial::ISurface *asSurface(Spatial::IGeometry *g) { return dynamic_cast<Spatial::ISurface *>(g); }
inline Spatial::IPolygon *asPolygon(Spatial::IGeometry *g) { return dynamic_cast<Spatial::IPolygon *>(g); }
inline Spatial::ITriangle *asTriangle(Spatial::IGeometry *g) { return dynamic_cast<Spatial::ITriangle *>(g); }
inline Spatial::IPolyhedralSurface *asPolyhedralSurface(Spatial::IGeometry *g) { return dynamic_cast<Spatial::IPolyhedralSurface *>(g); }
inline Spatial::ITIN *asTIN(Spatial::IGeometry *g) { return dynamic_cast<Spatial::ITIN *>(g); }
inline Spatial::IGeometryCollection *asGeometryCollection(Spatial::IGeometry *g) { return dynamic_cast<Spatial::IGeometryCollection *>(g); }
inline Spatial::IMultiPoint *asMultiPoint(Spatial::IGeometry *g) { return dynamic_cast<Spatial::IMultiPoint *>(g); }
inline Spatial::IMultiCurve *asMultiCurve(Spatial::IGeometry *g) { return dynamic_cast<Spatial::IMultiCurve *>(g); }
inline Spatial::IMultiLineString *asMultiLineString(Spatial::IGeometry *g) { return dynamic_cast<Spatial::IMultiLineString *>(g); }
inline Spatial::IMultiSurface *asMultiSurface(Spatial::IGeometry *g) { return dynamic_cast<Spatial::IMultiSurface *>(g); }
inline Spatial::IMultiPolygon *asMultiPolygon(Spatial::IGeometry *g) { return dynamic_cast<Spatial::IMultiPolygon *>(g); }

} // namespace Python
} // namespace HydroCouple

#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
#include <cstdlib>
#endif
#include <string>
#include <typeinfo>

namespace HydroCouple {
namespace Python {

/// A C++ type's readable name: demangled where the ABI mangles it (GCC,
/// Clang), as-is where it does not (MSVC already returns "class Foo").
inline std::string typeName(const std::type_info *type)
{
    if (!type)
        return {};
#if defined(__GNUC__) || defined(__clang__)
    int status = 0;
    char *readable = abi::__cxa_demangle(type->name(), nullptr, nullptr, &status);
    if (status == 0 && readable)
    {
        std::string name(readable);
        std::free(readable);
        return name;
    }
#endif
    return type->name();
}

} // namespace Python
} // namespace HydroCouple

namespace HydroCouple {
namespace Python {

inline Temporal::ITimeSeriesComponentDataItem *asTimeSeries(IComponentDataItem *item)
{
    return dynamic_cast<Temporal::ITimeSeriesComponentDataItem *>(item);
}

inline Temporal::ITimeIdBasedComponentDataItem *asTimeIdBased(IComponentDataItem *item)
{
    return dynamic_cast<Temporal::ITimeIdBasedComponentDataItem *>(item);
}

} // namespace Python
} // namespace HydroCouple

namespace HydroCouple {
namespace Python {

// -- Spatial and spatiotemporal data items ----------------------------------
inline Spatial::IGeometryComponentDataItem *asGeometryItem(IComponentDataItem *item) { return dynamic_cast<Spatial::IGeometryComponentDataItem *>(item); }
inline Spatial::INetworkComponentDataItem *asNetworkItem(IComponentDataItem *item) { return dynamic_cast<Spatial::INetworkComponentDataItem *>(item); }
inline Spatial::IPolyhedralSurfaceComponentDataItem *asPolyhedralSurfaceItem(IComponentDataItem *item) { return dynamic_cast<Spatial::IPolyhedralSurfaceComponentDataItem *>(item); }
inline Spatial::ITINComponentDataItem *asTINItem(IComponentDataItem *item) { return dynamic_cast<Spatial::ITINComponentDataItem *>(item); }
inline Spatial::IRasterComponentDataItem *asRasterItem(IComponentDataItem *item) { return dynamic_cast<Spatial::IRasterComponentDataItem *>(item); }
inline Spatial::IRegularGrid2DComponentDataItem *asRegularGrid2DItem(IComponentDataItem *item) { return dynamic_cast<Spatial::IRegularGrid2DComponentDataItem *>(item); }
inline Spatial::IRegularGrid3DComponentDataItem *asRegularGrid3DItem(IComponentDataItem *item) { return dynamic_cast<Spatial::IRegularGrid3DComponentDataItem *>(item); }

inline bool isTimeGeometryItem(IComponentDataItem *item) { return dynamic_cast<SpatioTemporal::ITimeGeometryComponentDataItem *>(item) != nullptr; }
inline bool isTimeNetworkItem(IComponentDataItem *item) { return dynamic_cast<SpatioTemporal::ITimeNetworkComponentDataItem *>(item) != nullptr; }
inline bool isTimePolyhedralSurfaceItem(IComponentDataItem *item) { return dynamic_cast<SpatioTemporal::ITimeSeriesPolyhedralSurfaceComponentDataItem *>(item) != nullptr; }
inline bool isTimeTINItem(IComponentDataItem *item) { return dynamic_cast<SpatioTemporal::ITimeSeriesTINComponentDataItem *>(item) != nullptr; }
inline bool isTimeRasterItem(IComponentDataItem *item) { return dynamic_cast<SpatioTemporal::ITimeSeriesRasterComponentDataItem *>(item) != nullptr; }
inline bool isTimeRegularGrid2DItem(IComponentDataItem *item) { return dynamic_cast<SpatioTemporal::ITimeRegularGrid2DComponentDataItem *>(item) != nullptr; }
inline bool isTimeRegularGrid3DItem(IComponentDataItem *item) { return dynamic_cast<SpatioTemporal::ITimeRegularGrid3DComponentDataItem *>(item) != nullptr; }
inline bool isTimeLayeredMeshItem(IComponentDataItem *item) { return dynamic_cast<SpatioTemporal::ITimeLayeredMeshComponentDataItem *>(item) != nullptr; }
inline bool isTimeLayeredNetworkItem(IComponentDataItem *item) { return dynamic_cast<SpatioTemporal::ITimeLayeredNetworkComponentDataItem *>(item) != nullptr; }

} // namespace Python
} // namespace HydroCouple

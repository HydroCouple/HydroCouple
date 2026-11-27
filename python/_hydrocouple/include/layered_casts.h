/*
 * layered_casts.h
 *
 * Cross-casts from a plain IComponentDataItem to the layered interfaces, for
 * the Python bindings. Same device as differential_bridge.h: Cython has no
 * dynamic_cast spelling, and a layered item reaches Python as whatever its
 * component handed out (an IOutput, an IInput, a result), so the binding has
 * to ask the object what else it is. A null result means "not that".
 *
 * ILayering is a mixin with no IComponentDataItem base, so these are true
 * cross-casts; dynamic_cast is the only cast that gets them right.
 */
#pragma once

#include "hydrocouple.h"
#include "hydrocouplespatial.h"
#include "hydrocouplespatiotemporal.h"

namespace HydroCouple {
namespace Python {

inline Spatial::ILayering *asLayering(IComponentDataItem *item)
{
    return dynamic_cast<Spatial::ILayering *>(item);
}

inline Spatial::ILayeredMeshComponentDataItem *asLayeredMesh(IComponentDataItem *item)
{
    return dynamic_cast<Spatial::ILayeredMeshComponentDataItem *>(item);
}

inline Spatial::ILayeredNetworkComponentDataItem *asLayeredNetwork(IComponentDataItem *item)
{
    return dynamic_cast<Spatial::ILayeredNetworkComponentDataItem *>(item);
}

inline SpatioTemporal::ITimeLayeredMeshComponentDataItem *
asTimeLayeredMesh(IComponentDataItem *item)
{
    return dynamic_cast<SpatioTemporal::ITimeLayeredMeshComponentDataItem *>(item);
}

inline SpatioTemporal::ITimeLayeredNetworkComponentDataItem *
asTimeLayeredNetwork(IComponentDataItem *item)
{
    return dynamic_cast<SpatioTemporal::ITimeLayeredNetworkComponentDataItem *>(item);
}

} // namespace Python
} // namespace HydroCouple

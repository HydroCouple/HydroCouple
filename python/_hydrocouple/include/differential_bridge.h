/*
 * differential_bridge.h
 *
 * Side-interface casts for the Python bindings (Phase G1). Cython has no
 * dynamic_cast spelling; these one-liners give it one. A null result means
 * the component does not implement the side interface -- the caller should
 * have consulted capabilities() first, and says so in its error.
 */
#pragma once

#include "hydrocouple.h"

namespace HydroCouple {
namespace Python {

inline IDifferentiableModelComponent *asDifferentiable(IModelComponent *component)
{
    return dynamic_cast<IDifferentiableModelComponent *>(component);
}

inline ICheckpointableModelComponent *asCheckpointable(IModelComponent *component)
{
    return dynamic_cast<ICheckpointableModelComponent *>(component);
}

} // namespace Python
} // namespace HydroCouple

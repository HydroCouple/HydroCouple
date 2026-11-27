# distutils: language = c++
# cython: language_level = 3
"""
Cython declarations for the C++ v2.0.0 interfaces in
``hydrocouplespatiotemporal.h``.

The spatiotemporal wrappers are the spatial item wrappers plus a time view,
so the bindings only need to ask a C++ item which spatiotemporal interface
it implements.
"""

from libcpp cimport bool as bint

cimport _hydrocouple._core as cpp


cdef extern from "interface_casts.h" namespace "HydroCouple::Python":
    bint isTimeGeometryItem(cpp.IComponentDataItem* item)
    bint isTimeNetworkItem(cpp.IComponentDataItem* item)
    bint isTimePolyhedralSurfaceItem(cpp.IComponentDataItem* item)
    bint isTimeTINItem(cpp.IComponentDataItem* item)
    bint isTimeRasterItem(cpp.IComponentDataItem* item)
    bint isTimeRegularGrid2DItem(cpp.IComponentDataItem* item)
    bint isTimeRegularGrid3DItem(cpp.IComponentDataItem* item)
    bint isTimeLayeredMeshItem(cpp.IComponentDataItem* item)
    bint isTimeLayeredNetworkItem(cpp.IComponentDataItem* item)

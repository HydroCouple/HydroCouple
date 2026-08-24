# distutils: language = c++
# cython: language_level = 3
"""
Cython declarations for the C++ v2.0.0 interfaces in
``hydrocouplespatiotemporal.h``.
"""

cimport _hydrocouple._core as cpp
cimport _hydrocouple._spatial as sp
cimport _hydrocouple._temporal as tmp


cdef extern from "hydrocouplespatiotemporal.h" namespace "HydroCouple::SpatioTemporal":

    cdef cppclass ITimeGeometryComponentDataItem(
            tmp.ITimeSeriesComponentDataItem):
        pass

    cdef cppclass ITimeNetworkComponentDataItem(
            tmp.ITimeSeriesComponentDataItem):
        pass

    cdef cppclass ITimeSeriesPolyhedralSurfaceComponentDataItem(
            tmp.ITimeSeriesComponentDataItem):
        pass

    cdef cppclass ITimeSeriesTINComponentDataItem(
            ITimeSeriesPolyhedralSurfaceComponentDataItem):
        sp.ITIN* TIN() const

    cdef cppclass ITimeSeriesRasterComponentDataItem(
            tmp.ITimeSeriesComponentDataItem):
        pass

    cdef cppclass ITimeRegularGrid2DComponentDataItem(
            tmp.ITimeSeriesComponentDataItem):
        pass

    cdef cppclass ITimeRegularGrid3DComponentDataItem(
            tmp.ITimeSeriesComponentDataItem):
        pass

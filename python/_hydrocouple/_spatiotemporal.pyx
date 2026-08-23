# distutils: language = c++
# cython: language_level = 3
"""
Cython wrapper classes for the C++ v2.0.0 spatiotemporal interfaces.

The spatiotemporal data items are typed combinations of the temporal and
spatial data items; data access goes through the core hyperslab API with
time as the outermost dimension.
"""

cimport _hydrocouple._core as cpp
cimport _hydrocouple._spatial as sp
cimport _hydrocouple._spatiotemporal as st
cimport _hydrocouple._temporal as tmp

from _hydrocouple._temporal import CppTimeSeriesComponentDataItemWrapper


cdef class CppTimeGeometryComponentDataItemWrapper:
    """Wrapper around a C++
    ``SpatioTemporal::ITimeGeometryComponentDataItem`` pointer.

    Canonical dimension ordering: time 0, geometry 1.
    """

    cdef st.ITimeGeometryComponentDataItem* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppTimeGeometryComponentDataItemWrapper wrap(
            st.ITimeGeometryComponentDataItem* ptr):
        cdef CppTimeGeometryComponentDataItemWrapper obj = (
            CppTimeGeometryComponentDataItemWrapper.__new__(
                CppTimeGeometryComponentDataItemWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this data item."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")

    @property
    def time_count(self) -> int:
        """The number of times."""
        return (<tmp.ITimeSeriesComponentDataItem*>self._ptr).timeCount()


cdef class CppTimeNetworkComponentDataItemWrapper:
    """Wrapper around a C++
    ``SpatioTemporal::ITimeNetworkComponentDataItem`` pointer.

    Canonical dimension ordering: time 0, entity 1.
    """

    cdef st.ITimeNetworkComponentDataItem* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppTimeNetworkComponentDataItemWrapper wrap(
            st.ITimeNetworkComponentDataItem* ptr):
        cdef CppTimeNetworkComponentDataItemWrapper obj = (
            CppTimeNetworkComponentDataItemWrapper.__new__(
                CppTimeNetworkComponentDataItemWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this data item."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")


cdef class CppTimeSeriesPolyhedralSurfaceComponentDataItemWrapper:
    """Wrapper around a C++
    ``SpatioTemporal::ITimeSeriesPolyhedralSurfaceComponentDataItem``
    pointer. Canonical dimension ordering: time 0, entity 1.
    """

    cdef st.ITimeSeriesPolyhedralSurfaceComponentDataItem* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppTimeSeriesPolyhedralSurfaceComponentDataItemWrapper wrap(
            st.ITimeSeriesPolyhedralSurfaceComponentDataItem* ptr):
        cdef CppTimeSeriesPolyhedralSurfaceComponentDataItemWrapper obj = (
            CppTimeSeriesPolyhedralSurfaceComponentDataItemWrapper.__new__(
                CppTimeSeriesPolyhedralSurfaceComponentDataItemWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this data item."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")


cdef class CppTimeSeriesTINComponentDataItemWrapper(
        CppTimeSeriesPolyhedralSurfaceComponentDataItemWrapper):
    """Wrapper around a C++
    ``SpatioTemporal::ITimeSeriesTINComponentDataItem`` pointer."""

    @staticmethod
    cdef CppTimeSeriesTINComponentDataItemWrapper wrap_tin(
            st.ITimeSeriesTINComponentDataItem* ptr):
        cdef CppTimeSeriesTINComponentDataItemWrapper obj = (
            CppTimeSeriesTINComponentDataItemWrapper.__new__(
                CppTimeSeriesTINComponentDataItemWrapper))
        obj._ptr = <st.ITimeSeriesPolyhedralSurfaceComponentDataItem*>ptr
        return obj


cdef class CppTimeSeriesRasterComponentDataItemWrapper:
    """Wrapper around a C++
    ``SpatioTemporal::ITimeSeriesRasterComponentDataItem`` pointer.

    Canonical dimension ordering: time 0, band 1, y 2, x 3.
    """

    cdef st.ITimeSeriesRasterComponentDataItem* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppTimeSeriesRasterComponentDataItemWrapper wrap(
            st.ITimeSeriesRasterComponentDataItem* ptr):
        cdef CppTimeSeriesRasterComponentDataItemWrapper obj = (
            CppTimeSeriesRasterComponentDataItemWrapper.__new__(
                CppTimeSeriesRasterComponentDataItemWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this data item."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")


cdef class CppTimeRegularGrid2DComponentDataItemWrapper:
    """Wrapper around a C++
    ``SpatioTemporal::ITimeRegularGrid2DComponentDataItem`` pointer.

    Canonical dimension ordering: time 0, y-cell 1, x-cell 2.
    """

    cdef st.ITimeRegularGrid2DComponentDataItem* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppTimeRegularGrid2DComponentDataItemWrapper wrap(
            st.ITimeRegularGrid2DComponentDataItem* ptr):
        cdef CppTimeRegularGrid2DComponentDataItemWrapper obj = (
            CppTimeRegularGrid2DComponentDataItemWrapper.__new__(
                CppTimeRegularGrid2DComponentDataItemWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this data item."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")


cdef class CppTimeRegularGrid3DComponentDataItemWrapper:
    """Wrapper around a C++
    ``SpatioTemporal::ITimeRegularGrid3DComponentDataItem`` pointer.

    Canonical dimension ordering: time 0, z-cell 1, y-cell 2, x-cell 3.
    """

    cdef st.ITimeRegularGrid3DComponentDataItem* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppTimeRegularGrid3DComponentDataItemWrapper wrap(
            st.ITimeRegularGrid3DComponentDataItem* ptr):
        cdef CppTimeRegularGrid3DComponentDataItemWrapper obj = (
            CppTimeRegularGrid3DComponentDataItemWrapper.__new__(
                CppTimeRegularGrid3DComponentDataItemWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this data item."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")


def _register_abc_subclasses():
    """Register wrappers with the spatiotemporal ABCs."""
    from hydrocouple.spatiotemporal import (
        ITimeGeometryComponentDataItem,
        ITimeNetworkComponentDataItem,
        ITimeRegularGrid2DComponentDataItem,
        ITimeRegularGrid3DComponentDataItem,
        ITimeSeriesPolyhedralSurfaceComponentDataItem,
        ITimeSeriesRasterComponentDataItem,
        ITimeSeriesTINComponentDataItem,
    )

    ITimeGeometryComponentDataItem.register(
        CppTimeGeometryComponentDataItemWrapper)
    ITimeNetworkComponentDataItem.register(
        CppTimeNetworkComponentDataItemWrapper)
    ITimeSeriesPolyhedralSurfaceComponentDataItem.register(
        CppTimeSeriesPolyhedralSurfaceComponentDataItemWrapper)
    ITimeSeriesTINComponentDataItem.register(
        CppTimeSeriesTINComponentDataItemWrapper)
    ITimeSeriesRasterComponentDataItem.register(
        CppTimeSeriesRasterComponentDataItemWrapper)
    ITimeRegularGrid2DComponentDataItem.register(
        CppTimeRegularGrid2DComponentDataItemWrapper)
    ITimeRegularGrid3DComponentDataItem.register(
        CppTimeRegularGrid3DComponentDataItemWrapper)


_register_abc_subclasses()

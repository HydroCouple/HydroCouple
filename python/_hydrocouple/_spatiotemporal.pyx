# distutils: language = c++
# cython: language_level = 3
"""
Wrapper classes for the C++ v2.0.0 spatiotemporal interfaces.

A spatiotemporal item is a spatial item whose values also vary in time, so
each wrapper here *is* the spatial item wrapper (subclassed) with the
time-series members delegated to a temporal view of the same C++ object.
Data access is the inherited hyperslab API with time as the outermost
dimension.

``as_spatiotemporal(item)`` and ``as_time_layered(item)`` give these views of
what the core bindings hand out for a loaded component's items.
"""

cimport _hydrocouple._core as cpp
from _hydrocouple._core cimport CppComponentDataItemWrapper
cimport _hydrocouple._spatiotemporal as st

from _hydrocouple._spatial import (
    CppGeometryComponentDataItemWrapper,
    CppLayeredMeshComponentDataItemWrapper,
    CppLayeredNetworkComponentDataItemWrapper,
    CppNetworkComponentDataItemWrapper,
    CppPolyhedralSurfaceComponentDataItemWrapper,
    CppRasterComponentDataItemWrapper,
    CppRegularGrid2DComponentDataItemWrapper,
    CppRegularGrid3DComponentDataItemWrapper,
    CppTINComponentDataItemWrapper,
    _make_view,
)
from _hydrocouple._temporal import as_time_series


def _delegate(name, doc):
    def get(self):
        return getattr(self._time_view, name)
    return property(get, doc=doc)


class _TimeView:
    """The ``ITimeSeriesComponentDataItem`` members, answered by a temporal
    view of the same C++ object (``self._time_view``)."""

    time_count = _delegate("time_count", "The number of times.")
    times = _delegate("times", "All time coordinates as Julian days "
                      "(float64, read-only, zero-copy).")
    time_span = _delegate("time_span", "The time span covered.")
    time_dimension = _delegate("time_dimension",
                               "The time dimension (dimension 0 of shape).")
    time_kind = _delegate("time_kind",
                          "What each value's time coordinate refers to.")
    interval_length = _delegate(
        "interval_length",
        "Averaging/accumulation interval in days; 0 for instantaneous.")
    time_interpolation = _delegate(
        "time_interpolation",
        "What the item does (output) or accepts (input) between instants.")
    time_extrapolation = _delegate(
        "time_extrapolation",
        "What the item does (output) or accepts (input) beyond its range.")

    def time(self, time_index):
        """The date/time at the given index (spot queries)."""
        return self._time_view.time(time_index)


class CppTimeGeometryComponentDataItemWrapper(
        _TimeView, CppGeometryComponentDataItemWrapper):
    """C++ ``SpatioTemporal::ITimeGeometryComponentDataItem``.
    Canonical dimension ordering: time 0, geometry 1."""


class CppTimeNetworkComponentDataItemWrapper(
        _TimeView, CppNetworkComponentDataItemWrapper):
    """C++ ``SpatioTemporal::ITimeNetworkComponentDataItem``.
    Canonical dimension ordering: time 0, entity 1."""


class CppTimeSeriesPolyhedralSurfaceComponentDataItemWrapper(
        _TimeView, CppPolyhedralSurfaceComponentDataItemWrapper):
    """C++ ``SpatioTemporal::ITimeSeriesPolyhedralSurfaceComponentDataItem``.
    Canonical dimension ordering: time 0, entity 1."""


class CppTimeSeriesTINComponentDataItemWrapper(
        _TimeView, CppTINComponentDataItemWrapper):
    """C++ ``SpatioTemporal::ITimeSeriesTINComponentDataItem``."""


class CppTimeSeriesRasterComponentDataItemWrapper(
        _TimeView, CppRasterComponentDataItemWrapper):
    """C++ ``SpatioTemporal::ITimeSeriesRasterComponentDataItem``.
    Canonical dimension ordering: time 0, band 1, y 2, x 3."""


class CppTimeRegularGrid2DComponentDataItemWrapper(
        _TimeView, CppRegularGrid2DComponentDataItemWrapper):
    """C++ ``SpatioTemporal::ITimeRegularGrid2DComponentDataItem``.
    Canonical dimension ordering: time 0, y-cell 1, x-cell 2."""


class CppTimeRegularGrid3DComponentDataItemWrapper(
        _TimeView, CppRegularGrid3DComponentDataItemWrapper):
    """C++ ``SpatioTemporal::ITimeRegularGrid3DComponentDataItem``.
    Canonical dimension ordering: time 0, z-cell 1, y-cell 2, x-cell 3."""


class CppTimeLayeredMeshComponentDataItemWrapper(
        _TimeView, CppLayeredMeshComponentDataItemWrapper):
    """C++ ``SpatioTemporal::ITimeLayeredMeshComponentDataItem``: the layered
    mesh accessors plus time. Canonical dimension ordering: time 0, entity
    1, layer 2."""


class CppTimeLayeredNetworkComponentDataItemWrapper(
        _TimeView, CppLayeredNetworkComponentDataItemWrapper):
    """C++ ``SpatioTemporal::ITimeLayeredNetworkComponentDataItem``: the
    layered network accessors (cross-sections included) plus time. Canonical
    dimension ordering: time 0, entity 1, layer 2."""


cdef object _view(cls, item):
    view = _make_view(cls, item)
    view._time_view = as_time_series(item)
    return view


cdef cpp.IComponentDataItem* _plain_pointer(object item) except? NULL:
    if not isinstance(item, CppComponentDataItemWrapper):
        raise TypeError(
            "expected a wrapped C++ data item (CppComponentDataItemWrapper, "
            f"CppInputWrapper, CppOutputWrapper, ...), got "
            f"{type(item).__name__}")
    return (<CppComponentDataItemWrapper>item)._ptr


def as_spatiotemporal(item):
    """The spatiotemporal view of a C++ data item, or ``None`` when it has
    none: the most specific of the wrappers in this module that the C++
    object implements. Accepts any data-item wrapper."""
    cdef cpp.IComponentDataItem* p = _plain_pointer(item)
    if p == NULL:
        return None
    if st.isTimeLayeredNetworkItem(p):
        return _view(CppTimeLayeredNetworkComponentDataItemWrapper, item)
    if st.isTimeLayeredMeshItem(p):
        return _view(CppTimeLayeredMeshComponentDataItemWrapper, item)
    if st.isTimeTINItem(p):
        return _view(CppTimeSeriesTINComponentDataItemWrapper, item)
    if st.isTimePolyhedralSurfaceItem(p):
        return _view(CppTimeSeriesPolyhedralSurfaceComponentDataItemWrapper,
                     item)
    if st.isTimeNetworkItem(p):
        return _view(CppTimeNetworkComponentDataItemWrapper, item)
    if st.isTimeGeometryItem(p):
        return _view(CppTimeGeometryComponentDataItemWrapper, item)
    if st.isTimeRasterItem(p):
        return _view(CppTimeSeriesRasterComponentDataItemWrapper, item)
    if st.isTimeRegularGrid3DItem(p):
        return _view(CppTimeRegularGrid3DComponentDataItemWrapper, item)
    if st.isTimeRegularGrid2DItem(p):
        return _view(CppTimeRegularGrid2DComponentDataItemWrapper, item)
    return None


def as_time_layered(item):
    """The time-varying layered view of a C++ data item, or ``None``.

    The counterpart of ``_hydrocouple._spatial.as_layered`` for the
    spatiotemporal items: returns a
    :class:`CppTimeLayeredNetworkComponentDataItemWrapper` or
    :class:`CppTimeLayeredMeshComponentDataItemWrapper`.
    """
    if not isinstance(item, CppComponentDataItemWrapper):
        plain = getattr(item, "data_item", None)
        if isinstance(plain, CppComponentDataItemWrapper):
            item = plain
    cdef cpp.IComponentDataItem* p = _plain_pointer(item)
    if p == NULL:
        return None
    if st.isTimeLayeredNetworkItem(p):
        return _view(CppTimeLayeredNetworkComponentDataItemWrapper, item)
    if st.isTimeLayeredMeshItem(p):
        return _view(CppTimeLayeredMeshComponentDataItemWrapper, item)
    return None


#: Every (ABC, wrapper) pair this module registers. A registration asserts
#: that the wrapper implements the ABC; tests/test_wrapper_conformance.py holds
#: each wrapper to it, since ABC.register() itself checks nothing.
ABC_REGISTRATIONS = []


def _register(abc_class, wrapper):
    abc_class.register(wrapper)
    ABC_REGISTRATIONS.append((abc_class, wrapper))


def _register_abc_subclasses():
    """Register wrappers with the spatiotemporal ABCs."""
    import hydrocouple.spatiotemporal as abcs

    _register(abcs.ITimeGeometryComponentDataItem,
              CppTimeGeometryComponentDataItemWrapper)
    _register(abcs.ITimeNetworkComponentDataItem,
              CppTimeNetworkComponentDataItemWrapper)
    _register(abcs.ITimeSeriesPolyhedralSurfaceComponentDataItem,
              CppTimeSeriesPolyhedralSurfaceComponentDataItemWrapper)
    _register(abcs.ITimeSeriesTINComponentDataItem,
              CppTimeSeriesTINComponentDataItemWrapper)
    _register(abcs.ITimeSeriesRasterComponentDataItem,
              CppTimeSeriesRasterComponentDataItemWrapper)
    _register(abcs.ITimeRegularGrid2DComponentDataItem,
              CppTimeRegularGrid2DComponentDataItemWrapper)
    _register(abcs.ITimeRegularGrid3DComponentDataItem,
              CppTimeRegularGrid3DComponentDataItemWrapper)
    _register(abcs.ITimeLayeredMeshComponentDataItem,
              CppTimeLayeredMeshComponentDataItemWrapper)
    _register(abcs.ITimeLayeredNetworkComponentDataItem,
              CppTimeLayeredNetworkComponentDataItemWrapper)


_register_abc_subclasses()

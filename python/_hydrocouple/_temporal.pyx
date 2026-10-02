# distutils: language = c++
# cython: language_level = 3
"""
Cython wrapper classes for the C++ v2.0.0 temporal interfaces.

``as_time_series(item)`` and ``as_time_model_component(component)`` give the
temporal view of what the core bindings hand out for a loaded component.
"""

from libcpp.vector cimport vector
from libcpp.string cimport string
from libc.stdint cimport int64_t

cimport _hydrocouple._core as cpp
from _hydrocouple._core cimport (
    CppComponentDataItemWrapper,
    CppModelComponentWrapper,
    CppPropertyChangedWrapper,
    bind_data_item,
    bind_identity,
    bind_signal,
    owned_by,
    wrap_dimension,
)
cimport _hydrocouple._temporal as tmp

import numpy as np
cimport numpy as cnp

cnp.import_array()


cdef object _span_to_readonly_array(tmp.const_double_span span):
    """Zero-copy read-only float64 view over a C++ span.

    Valid until the owning item's time dimension changes.
    """
    cdef size_t n = span.size()
    if n == 0:
        return np.empty(0, dtype=np.float64)
    cdef double[::1] mv = <double[:n]>(<double*>span.data())
    arr = np.asarray(mv)
    arr.flags.writeable = False
    return arr


# ---------------------------------------------------------------------------
# Dates and spans
# ---------------------------------------------------------------------------

cdef class CppDateTimeWrapper(CppPropertyChangedWrapper):
    """Wrapper around a C++ ``Temporal::IDateTime`` pointer."""

    # _ptr is declared in _temporal.pxd.

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppDateTimeWrapper wrap(tmp.IDateTime* ptr):
        cdef CppDateTimeWrapper obj = CppDateTimeWrapper.__new__(
            CppDateTimeWrapper)
        obj._ptr = ptr
        bind_signal(obj, <cpp.IPropertyChanged*>ptr)
        return obj

    @property
    def julian_day(self) -> float:
        """Date and time as a Julian day value."""
        return self._ptr.julianDay()

    @property
    def modified_julian_day(self) -> float:
        """Modified Julian day value."""
        return self._ptr.modifiedJulianDay()

    @property
    def serial_date(self) -> float:
        """Serial date number."""
        return self._ptr.serialDate()


cdef class CppTimeSpanWrapper(CppDateTimeWrapper):
    """Wrapper around a C++ ``Temporal::ITimeSpan`` pointer."""

    # _span is declared in _temporal.pxd.

    def __cinit__(self):
        self._span = NULL

    @property
    def duration(self) -> float:
        """Duration of the span in days."""
        return self._span.duration()

    @property
    def end_julian_day(self) -> float:
        """End of the span as a Julian day value."""
        return self._span.endJulianDay()


cdef object _wrap_date_time(const tmp.IDateTime* ptr):
    if ptr == NULL:
        return None
    return CppDateTimeWrapper.wrap(<tmp.IDateTime*>ptr)


cdef object wrap_time_span(tmp.ITimeSpan* ptr):
    if ptr == NULL:
        return None
    cdef CppTimeSpanWrapper obj = CppTimeSpanWrapper.__new__(
        CppTimeSpanWrapper)
    obj._ptr = <tmp.IDateTime*>ptr
    obj._span = ptr
    bind_signal(obj, <cpp.IPropertyChanged*>ptr)
    return obj


# ---------------------------------------------------------------------------
# Time-stepping components
# ---------------------------------------------------------------------------

cdef class CppTimeModelComponentWrapper(CppModelComponentWrapper):
    """Wrapper around a C++ ``Temporal::ITimeModelComponent``: the whole
    model-component wrapper plus the clock."""

    cdef tmp.ITimeModelComponent* _time

    def __cinit__(self):
        self._time = NULL

    @property
    def current_date_time(self):
        """Current simulation date/time."""
        return owned_by(_wrap_date_time(self._time.currentDateTime()), self)

    @property
    def simulation_period(self):
        """The time horizon of the model."""
        return owned_by(wrap_time_span(self._time.simulationPeriod()), self)

    @property
    def next_date_time_julian_day(self) -> float:
        """The time the next update() will advance to (Julian day)."""
        return self._time.nextDateTimeJulianDay()


def as_time_model_component(component):
    """The time-stepping view of a C++ component wrapper, or ``None`` when
    the component is not an ``ITimeModelComponent``. The view shares the
    original's ownership."""
    if not isinstance(component, CppModelComponentWrapper):
        raise TypeError(f"expected a C++ model component wrapper, got "
                        f"{type(component).__name__}")
    cdef cpp.IModelComponent* ptr = (<CppModelComponentWrapper>component)._ptr
    if ptr == NULL:
        return None
    cdef tmp.ITimeModelComponent* time = tmp.asTimeModelComponent(ptr)
    if time == NULL:
        return None
    cdef CppTimeModelComponentWrapper view = (
        CppTimeModelComponentWrapper.__new__(CppTimeModelComponentWrapper))
    view._ptr = ptr
    view._time = time
    bind_identity(view, <cpp.IIdentity*>ptr)
    return owned_by(view, component)


# ---------------------------------------------------------------------------
# Time-series data items
# ---------------------------------------------------------------------------

cdef class CppTimeSeriesComponentDataItemWrapper(CppComponentDataItemWrapper):
    """Wrapper around a C++ ``Temporal::ITimeSeriesComponentDataItem``: the
    data-item wrapper plus time and its declared semantics.

    Canonical dimension ordering: time is dimension 0 of ``shape``.
    """

    # _series is declared in _temporal.pxd.

    def __cinit__(self):
        self._series = NULL

    @staticmethod
    cdef CppTimeSeriesComponentDataItemWrapper wrap_series(
            tmp.ITimeSeriesComponentDataItem* ptr):
        cdef CppTimeSeriesComponentDataItemWrapper obj = (
            CppTimeSeriesComponentDataItemWrapper.__new__(
                CppTimeSeriesComponentDataItemWrapper))
        obj._series = ptr
        bind_data_item(obj, <cpp.IComponentDataItem*>ptr)
        return obj

    def time(self, time_index: int):
        """The date/time at the given index (spot queries)."""
        return owned_by(_wrap_date_time(self._series.time(<int64_t>time_index)),
                        self)

    @property
    def time_count(self) -> int:
        """The number of times."""
        return self._series.timeCount()

    @property
    def times(self):
        """Bulk Julian day values as a read-only float64 array (zero-copy;
        valid until the time dimension changes)."""
        return _span_to_readonly_array(self._series.times())

    @property
    def time_span(self):
        """The time span covered by this data item."""
        return owned_by(wrap_time_span(self._series.timeSpan()), self)

    @property
    def time_dimension(self):
        """The time dimension metadata."""
        return owned_by(wrap_dimension(self._series.timeDimension()), self)

    @property
    def time_kind(self):
        """What each value's time coordinate refers to."""
        from hydrocouple.temporal import TimeKind
        return TimeKind(<int>self._series.timeKind())

    @property
    def interval_length(self) -> float:
        """Averaging/accumulation interval in days; 0 for instantaneous."""
        return self._series.intervalLength()

    @property
    def time_interpolation(self):
        """What the item does (output) or accepts (input) between held
        instants."""
        from hydrocouple.temporal import TimeInterpolation
        return TimeInterpolation(<int>self._series.timeInterpolation())

    @property
    def time_extrapolation(self):
        """What the item does (output) or accepts (input) outside its held
        range."""
        from hydrocouple.temporal import TimeExtrapolation
        return TimeExtrapolation(<int>self._series.timeExtrapolation())


cdef class CppTimeIdBasedComponentDataItemWrapper(
        CppTimeSeriesComponentDataItemWrapper):
    """Wrapper around a C++ ``Temporal::ITimeIdBasedComponentDataItem``.

    Canonical dimension ordering: time 0, identifier 1.
    """

    # ITimeSeriesComponentDataItem is a virtual base of the id-based item,
    # so a typed pointer is stored (downcasting through a virtual base is
    # not possible).
    cdef tmp.ITimeIdBasedComponentDataItem* _id_ptr

    def __cinit__(self):
        self._id_ptr = NULL

    @property
    def identifiers(self) -> list:
        """The identifiers of the identifier dimension."""
        cdef vector[string] ids = self._id_ptr.identifiers()
        return [ids[i].decode("utf-8") for i in range(ids.size())]

    @property
    def identifier_dimension(self):
        """The identifier dimension (dimension 1 of ``shape``)."""
        return owned_by(wrap_dimension(self._id_ptr.identifierDimension()),
                        self)


cdef cpp.IComponentDataItem* _plain_pointer(object item) except? NULL:
    """The IComponentDataItem* behind any core data-item wrapper."""
    if not isinstance(item, CppComponentDataItemWrapper):
        raise TypeError(
            "expected a wrapped C++ data item (CppComponentDataItemWrapper, "
            f"CppInputWrapper, CppOutputWrapper, ...), got "
            f"{type(item).__name__}")
    return (<CppComponentDataItemWrapper>item)._ptr


def as_time_series(item):
    """The time-series view of a C++ data item, or ``None`` when it is not
    an ``ITimeSeriesComponentDataItem``. Accepts any data-item wrapper (an
    output, an input, a result); the view shares its ownership."""
    cdef cpp.IComponentDataItem* ptr = _plain_pointer(item)
    if ptr == NULL:
        return None
    cdef tmp.ITimeIdBasedComponentDataItem* id_based = tmp.asTimeIdBased(ptr)
    cdef CppTimeIdBasedComponentDataItemWrapper id_view
    if id_based != NULL:
        id_view = CppTimeIdBasedComponentDataItemWrapper.__new__(
            CppTimeIdBasedComponentDataItemWrapper)
        id_view._series = <tmp.ITimeSeriesComponentDataItem*>id_based
        id_view._id_ptr = id_based
        bind_data_item(id_view, ptr)
        return owned_by(id_view, item)
    cdef tmp.ITimeSeriesComponentDataItem* series = tmp.asTimeSeries(ptr)
    if series == NULL:
        return None
    return owned_by(CppTimeSeriesComponentDataItemWrapper.wrap_series(series),
                    item)


#: Every (ABC, wrapper) pair this module registers. A registration asserts
#: that the wrapper implements the ABC; tests/test_wrapper_conformance.py holds
#: each wrapper to it, since ABC.register() itself checks nothing.
ABC_REGISTRATIONS = []


def _register(abc_class, wrapper):
    abc_class.register(wrapper)
    ABC_REGISTRATIONS.append((abc_class, wrapper))


def _register_abc_subclasses():
    """Register wrappers with the temporal ABCs."""
    from hydrocouple.temporal import (
        IDateTime,
        ITimeIdBasedComponentDataItem,
        ITimeModelComponent,
        ITimeSeriesComponentDataItem,
        ITimeSpan,
    )

    _register(IDateTime, CppDateTimeWrapper)
    _register(ITimeSpan, CppTimeSpanWrapper)
    _register(ITimeModelComponent, CppTimeModelComponentWrapper)
    _register(ITimeSeriesComponentDataItem,
              CppTimeSeriesComponentDataItemWrapper)
    _register(ITimeIdBasedComponentDataItem,
              CppTimeIdBasedComponentDataItemWrapper)


_register_abc_subclasses()

# distutils: language = c++
# cython: language_level = 3
"""
Cython wrapper classes for the C++ v2.0.0 temporal interfaces.
"""

from libcpp.vector cimport vector
from libcpp.string cimport string
from libc.stdint cimport int64_t

cimport _hydrocouple._core as cpp
from _hydrocouple._core cimport CppDimensionWrapper
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


cdef class CppDateTimeWrapper:
    """Wrapper around a C++ ``Temporal::IDateTime`` pointer."""

    cdef tmp.IDateTime* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppDateTimeWrapper wrap(tmp.IDateTime* ptr):
        cdef CppDateTimeWrapper obj = CppDateTimeWrapper.__new__(
            CppDateTimeWrapper)
        obj._ptr = ptr
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


cdef class CppTimeSpanWrapper:
    """Wrapper around a C++ ``Temporal::ITimeSpan`` pointer."""

    cdef tmp.ITimeSpan* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppTimeSpanWrapper wrap(tmp.ITimeSpan* ptr):
        cdef CppTimeSpanWrapper obj = CppTimeSpanWrapper.__new__(
            CppTimeSpanWrapper)
        obj._ptr = ptr
        return obj

    @property
    def julian_day(self) -> float:
        """Start date and time as a Julian day value."""
        return self._ptr.julianDay()

    @property
    def modified_julian_day(self) -> float:
        """Modified Julian day value."""
        return self._ptr.modifiedJulianDay()

    @property
    def serial_date(self) -> float:
        """Serial date number."""
        return self._ptr.serialDate()

    @property
    def duration(self) -> float:
        """Duration of the timespan in days."""
        return self._ptr.duration()


cdef class CppTimeModelComponentWrapper:
    """Wrapper around a C++ ``Temporal::ITimeModelComponent`` pointer."""

    cdef tmp.ITimeModelComponent* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppTimeModelComponentWrapper wrap(tmp.ITimeModelComponent* ptr):
        cdef CppTimeModelComponentWrapper obj = (
            CppTimeModelComponentWrapper.__new__(
                CppTimeModelComponentWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this component."""
        return (<cpp.IModelComponent*>self._ptr).id().decode("utf-8")

    @property
    def current_date_time(self):
        """Current simulation date/time."""
        return CppDateTimeWrapper.wrap(self._ptr.currentDateTime())

    @property
    def simulation_period(self):
        """The time horizon of the model."""
        return CppTimeSpanWrapper.wrap(self._ptr.simulationPeriod())


cdef class CppTimeSeriesComponentDataItemWrapper:
    """Wrapper around a C++ ``Temporal::ITimeSeriesComponentDataItem``.

    Canonical dimension ordering: time is dimension 0 of ``shape``.
    """

    cdef tmp.ITimeSeriesComponentDataItem* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppTimeSeriesComponentDataItemWrapper wrap(
            tmp.ITimeSeriesComponentDataItem* ptr):
        cdef CppTimeSeriesComponentDataItemWrapper obj = (
            CppTimeSeriesComponentDataItemWrapper.__new__(
                CppTimeSeriesComponentDataItemWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this data item."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")

    def time(self, time_index: int):
        """The date/time at the given index (spot queries)."""
        return CppDateTimeWrapper.wrap(
            <tmp.IDateTime*>self._ptr.time(<int64_t>time_index))

    @property
    def time_count(self) -> int:
        """The number of times."""
        return self._ptr.timeCount()

    @property
    def times(self):
        """Bulk Julian day values as a read-only float64 array (zero-copy;
        valid until the time dimension changes)."""
        return _span_to_readonly_array(self._ptr.times())

    @property
    def time_span(self):
        """The time span covered by this data item."""
        return CppTimeSpanWrapper.wrap(self._ptr.timeSpan())

    @property
    def time_dimension(self):
        """The time dimension metadata."""
        return CppDimensionWrapper.wrap(self._ptr.timeDimension())


cdef class CppTimeIdBasedComponentDataItemWrapper(
        CppTimeSeriesComponentDataItemWrapper):
    """Wrapper around a C++ ``Temporal::ITimeIdBasedComponentDataItem``.

    Canonical dimension ordering: time 0, identifier 1.
    """

    # ITimeSeriesComponentDataItem is a virtual base of the id-based item,
    # so a typed pointer is stored (downcasting through a virtual base is
    # not possible).
    cdef tmp.ITimeIdBasedComponentDataItem* _id_ptr

    @staticmethod
    cdef CppTimeIdBasedComponentDataItemWrapper wrap_id_based(
            tmp.ITimeIdBasedComponentDataItem* ptr):
        cdef CppTimeIdBasedComponentDataItemWrapper obj = (
            CppTimeIdBasedComponentDataItemWrapper.__new__(
                CppTimeIdBasedComponentDataItemWrapper))
        obj._ptr = <tmp.ITimeSeriesComponentDataItem*>ptr
        obj._id_ptr = ptr
        return obj

    @property
    def identifiers(self) -> list:
        """The identifiers of the identifier dimension."""
        cdef vector[string] ids = self._id_ptr.identifiers()
        return [ids[i].decode("utf-8") for i in range(ids.size())]

    @property
    def identifier_dimension(self):
        """The identifier dimension (dimension 1 of ``shape``)."""
        return CppDimensionWrapper.wrap(self._id_ptr.identifierDimension())


def _register_abc_subclasses():
    """Register wrappers with the temporal ABCs."""
    from hydrocouple.temporal import (
        IDateTime,
        ITimeIdBasedComponentDataItem,
        ITimeModelComponent,
        ITimeSeriesComponentDataItem,
        ITimeSpan,
    )

    IDateTime.register(CppDateTimeWrapper)
    ITimeSpan.register(CppTimeSpanWrapper)
    ITimeModelComponent.register(CppTimeModelComponentWrapper)
    ITimeSeriesComponentDataItem.register(
        CppTimeSeriesComponentDataItemWrapper)
    ITimeIdBasedComponentDataItem.register(
        CppTimeIdBasedComponentDataItemWrapper)


_register_abc_subclasses()

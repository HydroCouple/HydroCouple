# distutils: language = c++
# cython: language_level = 3
"""
Cython declarations for the C++ v2.0.0 interfaces in ``hydrocoupletemporal.h``.
"""

from libcpp.vector cimport vector
from libcpp.string cimport string
from libc.stdint cimport int64_t, uint8_t

cimport _hydrocouple._core as cpp
from _hydrocouple._core cimport (
    CppComponentDataItemWrapper,
    CppModelComponentWrapper,
    CppPropertyChangedWrapper,
)


cdef extern from "<span>" namespace "std":
    cdef cppclass const_double_span "std::span<const double>":
        const_double_span()
        const double* data() const
        size_t size() const
        bint empty() const


cdef extern from "hydrocoupletemporal.h" namespace "HydroCouple::Temporal":

    cdef enum class TimeKind(uint8_t):
        Unknown
        Instantaneous
        IntervalMean
        IntervalMinimum
        IntervalMaximum
        Accumulated

    # C++ spells the second enumerator None, which Cython cannot.
    cdef enum class TimeInterpolation(uint8_t):
        Unknown
        NoInterpolation "HydroCouple::Temporal::TimeInterpolation::None"
        Previous
        Nearest
        Linear

    cdef enum class TimeExtrapolation(uint8_t):
        Unknown
        Refuse
        HoldLast
        Linear

    cdef cppclass IDateTime(cpp.IPropertyChanged):
        double julianDay() const
        double modifiedJulianDay() const
        double serialDate() const

    cdef cppclass ITimeSpan(IDateTime):
        double duration() const
        double endJulianDay() const

    cdef cppclass ITimeModelComponent(cpp.IModelComponent):
        IDateTime* currentDateTime() const
        ITimeSpan* simulationPeriod() const
        double nextDateTimeJulianDay() const

    cdef cppclass ITimeSeriesComponentDataItem(cpp.IComponentDataItem):
        const IDateTime* time(int64_t timeIndex) const
        int64_t timeCount() const
        const_double_span times() const
        ITimeSpan* timeSpan() const
        cpp.IDimension* timeDimension() const
        TimeKind timeKind() const
        double intervalLength() const
        TimeInterpolation timeInterpolation() const
        TimeExtrapolation timeExtrapolation() const

    cdef cppclass ITimeIdBasedComponentDataItem(ITimeSeriesComponentDataItem):
        vector[string] identifiers() const
        cpp.IDimension* identifierDimension() const


cdef extern from "interface_casts.h" namespace "HydroCouple::Python":
    ITimeModelComponent* asTimeModelComponent(cpp.IModelComponent* component)
    ITimeSeriesComponentDataItem* asTimeSeries(cpp.IComponentDataItem* item)
    ITimeIdBasedComponentDataItem* asTimeIdBased(cpp.IComponentDataItem* item)


# ---------------------------------------------------------------------------
# Shared cdef classes (implemented in _temporal.pyx)
# ---------------------------------------------------------------------------
cdef class CppDateTimeWrapper(CppPropertyChangedWrapper):
    cdef IDateTime* _ptr

    @staticmethod
    cdef CppDateTimeWrapper wrap(IDateTime* ptr)


cdef class CppTimeSpanWrapper(CppDateTimeWrapper):
    cdef ITimeSpan* _span


cdef class CppTimeSeriesComponentDataItemWrapper(CppComponentDataItemWrapper):
    cdef ITimeSeriesComponentDataItem* _series

    @staticmethod
    cdef CppTimeSeriesComponentDataItemWrapper wrap_series(
            ITimeSeriesComponentDataItem* ptr)


cdef object wrap_time_span(ITimeSpan* ptr)

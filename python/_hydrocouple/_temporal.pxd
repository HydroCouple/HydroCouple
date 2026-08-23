# distutils: language = c++
# cython: language_level = 3
"""
Cython declarations for the C++ v2.0.0 interfaces in ``hydrocoupletemporal.h``.
"""

from libcpp.vector cimport vector
from libcpp.string cimport string
from libc.stdint cimport int64_t

cimport _hydrocouple._core as cpp


cdef extern from "<span>" namespace "std":
    cdef cppclass const_double_span "std::span<const double>":
        const_double_span()
        const double* data() const
        size_t size() const
        bint empty() const


cdef extern from "hydrocoupletemporal.h" namespace "HydroCouple::Temporal":

    cdef cppclass IDateTime(cpp.IPropertyChanged):
        double julianDay() const
        double modifiedJulianDay() const
        double serialDate() const

    cdef cppclass ITimeSpan(IDateTime):
        double duration() const

    cdef cppclass ITimeModelComponent(cpp.IModelComponent):
        IDateTime* currentDateTime() const
        ITimeSpan* simulationPeriod() const

    cdef cppclass ITimeSeriesComponentDataItem(cpp.IComponentDataItem):
        const IDateTime* time(int64_t timeIndex) const
        int64_t timeCount() const
        const_double_span times() const
        ITimeSpan* timeSpan() const
        cpp.IDimension* timeDimension() const

    cdef cppclass ITimeIdBasedComponentDataItem(ITimeSeriesComponentDataItem):
        vector[string] identifiers() const
        cpp.IDimension* identifierDimension() const

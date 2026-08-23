# distutils: language = c++
# cython: language_level = 3
"""
Test-support extension: pure C++ consumption of bridged Python components.

The functions here call the C++ harness in ``cpp_test_harness.h``, which
drives components strictly through the ``HydroCouple::IModelComponent*``
virtual interface — proving that a Python component wrapped in
``PyComponentBridge`` is indistinguishable from a native C++ component,
data plane included.
"""

from libcpp.string cimport string
from libcpp.vector cimport vector
from libc.stdint cimport int32_t, int64_t

cimport _hydrocouple._core as cpp
from _hydrocouple._core cimport PyComponentBridge

import numpy as np
cimport numpy as cnp

cnp.import_array()


cdef extern from "cpp_test_harness.h" namespace "HydroCouple::Python::Testing":
    string driveLifecycle(cpp.IModelComponent* component) except +
    vector[int32_t] queryCapabilities(cpp.IModelComponent* component) except +
    vector[string] drainErrors(cpp.IModelComponent* component) except +
    bint readResultSlab(cpp.IModelComponent* component, int resultIndex,
                        const int64_t* start, const int64_t* count,
                        int32_t rank, double* out, string& message) except +
    bint writeResultSlab(cpp.IModelComponent* component, int resultIndex,
                         const int64_t* start, const int64_t* count,
                         int32_t rank, const double* values,
                         string& message) except +


def cpp_drive_lifecycle(PyComponentBridge bridge) -> str:
    """Run the full lifecycle from C++; returns the C++-observed trace."""
    return driveLifecycle(bridge.ptr()).decode("utf-8")


def cpp_query_capabilities(PyComponentBridge bridge) -> list:
    """The capability values C++ observes through capabilities()."""
    cdef vector[int32_t] caps = queryCapabilities(bridge.ptr())
    return sorted(caps[i] for i in range(caps.size()))


def cpp_drain_errors(PyComponentBridge bridge) -> list:
    """Drain errors() from C++; rows are 'severity|code|source|message'."""
    cdef vector[string] rows = drainErrors(bridge.ptr())
    return [rows[i].decode("utf-8") for i in range(rows.size())]


def cpp_read_result(PyComponentBridge bridge, int result_index,
                    start, count):
    """Read a Float64 hyperslab from C++ into a fresh ndarray.

    The BufferDescriptor is built in C++ over C++-owned memory; the values
    land in Python only after the C++ read completes.
    :returns: ``(ok, values, message)``.
    """
    cdef cnp.ndarray[int64_t, ndim=1] s = np.asarray(start, dtype=np.int64)
    cdef cnp.ndarray[int64_t, ndim=1] c = np.asarray(count, dtype=np.int64)
    cdef cnp.ndarray[double, ndim=1] out = np.zeros(
        int(np.prod(np.asarray(count))), dtype=np.float64)
    cdef string msg
    cdef bint ok = readResultSlab(
        bridge.ptr(), result_index,
        <const int64_t*>s.data, <const int64_t*>c.data,
        <int32_t>s.shape[0], <double*>out.data, msg)
    return bool(ok), out.reshape(tuple(count)), msg.decode("utf-8")


def cpp_write_result(PyComponentBridge bridge, int result_index,
                     values, start, count):
    """Write a Float64 hyperslab from C++-owned memory into the component.

    :returns: ``(ok, message)``.
    """
    cdef cnp.ndarray[int64_t, ndim=1] s = np.asarray(start, dtype=np.int64)
    cdef cnp.ndarray[int64_t, ndim=1] c = np.asarray(count, dtype=np.int64)
    cdef cnp.ndarray[double, ndim=1] vals = np.ascontiguousarray(
        np.asarray(values, dtype=np.float64).ravel())
    cdef string msg
    cdef bint ok = writeResultSlab(
        bridge.ptr(), result_index,
        <const int64_t*>s.data, <const int64_t*>c.data,
        <int32_t>s.shape[0], <const double*>vals.data, msg)
    return bool(ok), msg.decode("utf-8")

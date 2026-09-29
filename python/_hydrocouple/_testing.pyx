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


# ---------------------------------------------------------------------------
# Phase G0: DLPack gates
# ---------------------------------------------------------------------------

from libcpp cimport bool as cbool
from libc.stdint cimport uintptr_t, uint64_t
from cpython.ref cimport PyObject

from _hydrocouple._core cimport CppComponentDataItemWrapper
cimport _hydrocouple._dlpack as dl


cdef extern from "cpp_test_harness.h" namespace "HydroCouple::Python::Testing":
    cdef cppclass ProbeRecord:
        uintptr_t address
        int kind
        int32_t rank
        vector[int64_t] shape
        vector[int64_t] stridesBytes
        cbool stridesNull
        int space
        int32_t deviceId
        int calls

    cdef cppclass ProbeDataItem(cpp.IComponentDataItem):
        ProbeDataItem()
        ProbeRecord last
        double value(int64_t i, int64_t j) const

    bint callResultWithDescriptor(
        cpp.IModelComponent* component, int resultIndex, bint read,
        uintptr_t address, int kind, const vector[int64_t]& shape,
        const vector[int64_t]& stridesBytes, bint stridesNull, int space,
        int32_t deviceId, const vector[int64_t]& start,
        const vector[int64_t]& count, string& message) except +


cdef class Probe:
    """Owns a native ProbeDataItem; ``item`` is the ordinary C++ wrapper
    Python code would get from a loaded component."""

    cdef ProbeDataItem* _probe
    cdef object _item

    def __cinit__(self):
        self._probe = new ProbeDataItem()
        self._item = CppComponentDataItemWrapper.wrap(
            <cpp.IComponentDataItem*>self._probe)

    def __dealloc__(self):
        del self._probe

    @property
    def item(self):
        return self._item

    @property
    def last(self) -> dict:
        """The descriptor the probe saw on its last data-plane call."""
        cdef ProbeRecord* r = &self._probe.last
        return {
            "address": int(r.address),
            "kind": int(r.kind),
            "rank": int(r.rank),
            "shape": tuple(r.shape[i] for i in range(r.shape.size())),
            "strides_bytes": (None if r.stridesNull else tuple(
                r.stridesBytes[i] for i in range(r.stridesBytes.size()))),
            "space": int(r.space),
            "device_id": int(r.deviceId),
            "calls": int(r.calls),
        }

    def values(self):
        """The probe's own storage, copied out through C++ (not the data
        plane under test)."""
        out = np.empty((3, 4), dtype=np.float64)
        for i in range(3):
            for j in range(4):
                out[i, j] = self._probe.value(i, j)
        return out


def cpp_call_result(PyComponentBridge bridge, int result_index, bint read,
                    uintptr_t address, int kind, shape, strides_bytes,
                    int space, int device_id, start, count):
    """Call a Python result item from C++ with a raw descriptor.

    ``strides_bytes=None`` passes a null stride pointer. Nothing here
    dereferences ``address``; only the Python item might.
    :returns: ``(ok, message)``.
    """
    cdef vector[int64_t] sh, st, s0, c0
    for v in shape:
        sh.push_back(v)
    if strides_bytes is not None:
        for v in strides_bytes:
            st.push_back(v)
    for v in start:
        s0.push_back(v)
    for v in count:
        c0.push_back(v)
    cdef string msg
    cdef bint ok = callResultWithDescriptor(
        bridge.ptr(), result_index, read, address, kind, sh, st,
        strides_bytes is None, space, device_id, s0, c0, msg)
    return bool(ok), msg.decode("utf-8")


def inspect_dlpack(producer, versioned=True) -> dict:
    """Consume one capsule from ``producer`` and report its DLTensor.

    Runs the producer's deleter before returning, as any consumer must.
    """
    if versioned:
        capsule = producer.__dlpack__(max_version=(1, 3))
    else:
        capsule = producer.__dlpack__()
    cdef dl.DLPackBorrow borrow
    cdef string msg
    if not dl.borrowCapsule(<PyObject*>capsule, borrow, msg):
        raise BufferError(msg.decode("utf-8"))
    cdef const dl.DLTensor* t = borrow.tensor()
    info = {
        "data": <uintptr_t>t.data,
        "device": (int(t.device.device_type), int(t.device.device_id)),
        "ndim": int(t.ndim),
        "dtype": (int(t.dtype.code), int(t.dtype.bits), int(t.dtype.lanes)),
        "shape": tuple(t.shape[i] for i in range(t.ndim)),
        "strides": (None if t.strides == NULL
                    else tuple(t.strides[i] for i in range(t.ndim))),
        "byte_offset": int(t.byte_offset),
        "flags": int(borrow.flags()),
        "capsule": ("used_dltensor_versioned" if versioned
                    else "used_dltensor"),
    }
    borrow.release()
    return info

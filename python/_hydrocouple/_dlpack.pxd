# distutils: language = c++
# cython: language_level = 3
"""
Cython declarations for ``dlpack_bridge.h`` (DLPack <-> BufferDescriptor).

Extern declarations only; there is no ``_dlpack`` extension module. The
bindings speak DLPack; the interface headers never see it.
"""

from libcpp.string cimport string
from libc.stdint cimport int32_t, int64_t, uint64_t
from cpython.ref cimport PyObject

cimport _hydrocouple._core as cpp


cdef extern from "hc_dlpack.h":
    ctypedef enum DLDeviceType:
        pass

    ctypedef struct DLDevice:
        DLDeviceType device_type
        int32_t device_id

    ctypedef struct DLDataType:
        unsigned char code
        unsigned char bits
        unsigned short lanes

    ctypedef struct DLTensor:
        void* data
        DLDevice device
        int32_t ndim
        DLDataType dtype
        int64_t* shape
        int64_t* strides
        uint64_t byte_offset

    int kDLCPU
    int kDLCUDA
    int kDLCUDAHost
    int kDLROCM
    int kDLROCMHost
    int kDLCUDAManaged
    int kDLOneAPI
    int kDLMetal
    int DLPACK_MAJOR_VERSION
    int DLPACK_MINOR_VERSION
    uint64_t DLPACK_FLAG_BITMASK_READ_ONLY


cdef extern from "dlpack_bridge.h" namespace "HydroCouple::Python::DLPack":
    cdef cppclass DLPackBorrow:
        DLPackBorrow()
        const DLTensor* tensor() const
        uint64_t flags() const
        void release()

    bint borrowCapsule(PyObject* capsule, DLPackBorrow& borrow,
                       string& message)
    bint descriptorFromBorrow(DLPackBorrow& borrow, bint writable,
                              cpp.BufferDescriptor& descriptor,
                              string& message)
    PyObject* makeCapsule(const cpp.BufferDescriptor& descriptor,
                          DLDevice device, bint readOnly, bint versioned,
                          string& message, uint64_t byteOffset)
    DLDevice deviceFromSpace(cpp.MemorySpace space, int32_t deviceId)
    bint spaceFromDeviceType(int32_t deviceType, cpp.MemorySpace& space)
    int32_t accelerator()
    bint setAccelerator(int32_t deviceType)
    int64_t liveExports()
    cpp.DataKind kindFromDLDataType(DLDataType type)

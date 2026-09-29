/*
 * dlpack_bridge.h
 *
 * DLPack <-> BufferDescriptor, for the Python bindings only (Phase G0 of
 * plans/hydrocouple/DIFFERENTIABLE_INTERFACE_ALTERNATIVES_2026-09-28.md).
 *
 * A HydroCouple BufferDescriptor and a DLPack DLTensor describe the same
 * thing -- a typed, strided, possibly device-resident array that neither of
 * them owns -- so moving between them is a field-for-field translation with
 * three real decisions in it, each of which is made here and nowhere else:
 *
 *   1. Strides. DLPack counts elements; BufferDescriptor counts bytes.
 *
 *   2. Devices. DLPack names a vendor (CUDA, ROCm, oneAPI, Metal, ...);
 *      MemorySpace is vendor-neutral by design. Importing forgets the vendor
 *      (every accelerator is MemorySpace::Device). Exporting needs one back,
 *      and takes it from a process-wide "accelerator" the bindings are told
 *      about (setAccelerator), which defaults to CUDA.
 *
 *   3. Offsets. DLPack carries byte_offset; BufferDescriptor does not. For
 *      devices whose data pointer is an address (CPU, CUDA, ROCm, oneAPI) the
 *      offset is folded into the pointer. For devices whose data pointer is
 *      an opaque handle (OpenCL cl_mem, Vulkan, Metal, WebGPU) that would be
 *      pointer arithmetic on a handle, so a non-zero offset is refused.
 *
 * Lifetime. A consumed capsule is renamed "used_dltensor[_versioned]" as the
 * protocol requires, and the borrowed tensor's deleter runs exactly once --
 * when the DLPackBorrow that holds it is destroyed. Capsules this header
 * produces count themselves in liveExports() until their deleter runs, so a
 * test can prove that nothing leaks.
 *
 * Everything here needs Python.h and hc_dlpack.h, which is precisely why it
 * is not in the interface.
 */
#pragma once

#include <Python.h>

#include "hc_dlpack.h"
#include "hydrocouple.h"

#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

namespace HydroCouple {
namespace Python {
namespace DLPack {

// ---------------------------------------------------------------------------
// Element types
// ---------------------------------------------------------------------------

/// The DataKind a DLPack element type denotes, or DataKind::Unknown when the
/// standard has no such kind (bfloat16, complex, vector lanes, fp8, ...).
inline DataKind kindFromDLDataType(DLDataType type)
{
    if (type.lanes != 1)
        return DataKind::Unknown;
    switch (type.code)
    {
        case kDLInt:
            switch (type.bits)
            {
                case 8:  return DataKind::Int8;
                case 16: return DataKind::Int16;
                case 32: return DataKind::Int32;
                case 64: return DataKind::Int64;
                default: return DataKind::Unknown;
            }
        case kDLUInt:
            switch (type.bits)
            {
                case 8:  return DataKind::UInt8;
                case 16: return DataKind::UInt16;
                case 32: return DataKind::UInt32;
                case 64: return DataKind::UInt64;
                default: return DataKind::Unknown;
            }
        case kDLFloat:
            switch (type.bits)
            {
                case 32: return DataKind::Float32;
                case 64: return DataKind::Float64;
                default: return DataKind::Unknown;
            }
        case kDLBool:
            return type.bits == 8 ? DataKind::Boolean : DataKind::Unknown;
        default:
            return DataKind::Unknown;
    }
}

/// The DLPack element type for a DataKind. False for kinds DLPack cannot
/// carry (Unknown, String, Opaque).
inline bool dlDataTypeFromKind(DataKind kind, DLDataType &type)
{
    type.lanes = 1;
    switch (kind)
    {
        case DataKind::Int8:    type.code = kDLInt;   type.bits = 8;  return true;
        case DataKind::UInt8:   type.code = kDLUInt;  type.bits = 8;  return true;
        case DataKind::Int16:   type.code = kDLInt;   type.bits = 16; return true;
        case DataKind::UInt16:  type.code = kDLUInt;  type.bits = 16; return true;
        case DataKind::Int32:   type.code = kDLInt;   type.bits = 32; return true;
        case DataKind::UInt32:  type.code = kDLUInt;  type.bits = 32; return true;
        case DataKind::Int64:   type.code = kDLInt;   type.bits = 64; return true;
        case DataKind::UInt64:  type.code = kDLUInt;  type.bits = 64; return true;
        case DataKind::Float32: type.code = kDLFloat; type.bits = 32; return true;
        case DataKind::Float64: type.code = kDLFloat; type.bits = 64; return true;
        case DataKind::Boolean: type.code = kDLBool;  type.bits = 8;  return true;
        default:                return false;
    }
}

/// Bytes per element for the kinds DLPack can carry; 0 otherwise.
inline int64_t itemSize(DataKind kind)
{
    DLDataType type;
    if (!dlDataTypeFromKind(kind, type))
        return 0;
    return type.bits / 8;
}

// ---------------------------------------------------------------------------
// Devices
// ---------------------------------------------------------------------------

/// Whether a device's data pointer is an opaque handle rather than an
/// address, so that byte_offset cannot be folded into it.
inline bool isOpaqueHandleDevice(int32_t deviceType)
{
    return deviceType == kDLOpenCL || deviceType == kDLVulkan ||
           deviceType == kDLMetal || deviceType == kDLWebGPU;
}

/// The MemorySpace a DLPack device type denotes. False for a device type
/// this header does not know (a newer DLPack minor version's addition).
inline bool spaceFromDeviceType(int32_t deviceType, MemorySpace &space)
{
    switch (deviceType)
    {
        case kDLCPU:
            space = MemorySpace::Host;
            return true;
        case kDLCUDAHost:
        case kDLROCMHost:
            space = MemorySpace::HostPinned;
            return true;
        case kDLCUDAManaged:
            space = MemorySpace::Unified;
            return true;
        case kDLCUDA:
        case kDLOpenCL:
        case kDLVulkan:
        case kDLMetal:
        case kDLVPI:
        case kDLROCM:
        case kDLExtDev:
        case kDLOneAPI:
        case kDLWebGPU:
        case kDLHexagon:
        case kDLMAIA:
        case kDLTrn:
            space = MemorySpace::Device;
            return true;
        default:
            return false;
    }
}

/// The accelerator that MemorySpace::Device means when exporting. Process
/// wide, because MemorySpace deliberately does not say.
inline std::atomic<int32_t> &acceleratorSlot()
{
    static std::atomic<int32_t> slot{kDLCUDA};
    return slot;
}

inline int32_t accelerator() { return acceleratorSlot().load(); }

/// Set the accelerator. False (and unchanged) unless deviceType is one that
/// spaceFromDeviceType maps to MemorySpace::Device.
inline bool setAccelerator(int32_t deviceType)
{
    MemorySpace space;
    if (!spaceFromDeviceType(deviceType, space) || space != MemorySpace::Device)
        return false;
    acceleratorSlot().store(deviceType);
    return true;
}

/// The DLPack device for a descriptor's (space, deviceId), resolved against
/// the current accelerator.
inline DLDevice deviceFromSpace(MemorySpace space, int32_t deviceId)
{
    const int32_t acc = accelerator();
    DLDevice device;
    device.device_id = deviceId;
    switch (space)
    {
        case MemorySpace::Host:
            device.device_type = kDLCPU;
            device.device_id = 0;
            break;
        case MemorySpace::HostPinned:
            // Pinned memory is host memory; only CUDA and ROCm name it.
            device.device_type = acc == kDLCUDA ? kDLCUDAHost
                               : acc == kDLROCM ? kDLROCMHost
                                                : kDLCPU;
            if (device.device_type == kDLCPU)
                device.device_id = 0;
            break;
        case MemorySpace::Unified:
            // Only CUDA has a managed-memory device type; elsewhere unified
            // memory is addressed as the accelerator's own.
            device.device_type = acc == kDLCUDA ? kDLCUDAManaged
                                                : static_cast<DLDeviceType>(acc);
            break;
        case MemorySpace::Device:
        default:
            device.device_type = static_cast<DLDeviceType>(acc);
            break;
    }
    return device;
}

// ---------------------------------------------------------------------------
// Consuming: a capsule in, a BufferDescriptor out
// ---------------------------------------------------------------------------

/// A borrowed DLPack tensor. Holds the managed tensor taken out of a
/// capsule, and the byte-stride storage the descriptor points at; runs the
/// producer's deleter exactly once, on release() or destruction.
struct DLPackBorrow
{
    DLManagedTensorVersioned *versioned = nullptr;
    DLManagedTensor *legacy = nullptr;
    std::vector<int64_t> stridesBytes;

    DLPackBorrow() = default;
    DLPackBorrow(const DLPackBorrow &) = delete;
    DLPackBorrow &operator=(const DLPackBorrow &) = delete;
    ~DLPackBorrow() { release(); }

    const DLTensor *tensor() const
    {
        if (versioned)
            return &versioned->dl_tensor;
        if (legacy)
            return &legacy->dl_tensor;
        return nullptr;
    }

    uint64_t flags() const { return versioned ? versioned->flags : 0; }

    void release()
    {
        if (versioned)
        {
            DLManagedTensorVersioned *t = versioned;
            versioned = nullptr;
            if (t->deleter)
                t->deleter(t);
        }
        if (legacy)
        {
            DLManagedTensor *t = legacy;
            legacy = nullptr;
            if (t->deleter)
                t->deleter(t);
        }
    }
};

/// Take the managed tensor out of a DLPack capsule. On success the capsule
/// is renamed "used_..." and `borrow` owns the deleter. Requires the GIL.
inline bool borrowCapsule(PyObject *capsule, DLPackBorrow &borrow,
                          std::string &message)
{
    if (PyCapsule_IsValid(capsule, "dltensor_versioned"))
    {
        auto *t = static_cast<DLManagedTensorVersioned *>(
            PyCapsule_GetPointer(capsule, "dltensor_versioned"));
        if (t == nullptr)
        {
            message = "DLPack capsule holds no tensor";
            return false;
        }
        if (PyCapsule_SetName(capsule, "used_dltensor_versioned") != 0)
        {
            message = "DLPack capsule could not be marked consumed";
            return false;
        }
        borrow.versioned = t;
        const uint32_t major = t->version.major;
        if (major != DLPACK_MAJOR_VERSION)
        {
            // The protocol: a major mismatch means only the deleter is safe.
            borrow.release();
            message = "DLPack major version " + std::to_string(major) +
                      " is not " + std::to_string(DLPACK_MAJOR_VERSION) +
                      "; the producer's tensor was released unread";
            return false;
        }
        return true;
    }
    if (PyCapsule_IsValid(capsule, "dltensor"))
    {
        auto *t = static_cast<DLManagedTensor *>(
            PyCapsule_GetPointer(capsule, "dltensor"));
        if (t == nullptr)
        {
            message = "DLPack capsule holds no tensor";
            return false;
        }
        if (PyCapsule_SetName(capsule, "used_dltensor") != 0)
        {
            message = "DLPack capsule could not be marked consumed";
            return false;
        }
        borrow.legacy = t;
        return true;
    }
    message = "not a DLPack capsule, or one that was already consumed";
    return false;
}

/// Describe a borrowed tensor as a BufferDescriptor. The descriptor points
/// into `borrow` (strides) and into the tensor (shape), so it is valid only
/// while `borrow` is. `writable` refuses tensors the producer marked
/// read-only.
inline bool descriptorFromBorrow(DLPackBorrow &borrow, bool writable,
                                 BufferDescriptor &descriptor,
                                 std::string &message)
{
    const DLTensor *t = borrow.tensor();
    if (t == nullptr)
    {
        message = "no DLPack tensor is borrowed";
        return false;
    }
    if (writable && (borrow.flags() & DLPACK_FLAG_BITMASK_READ_ONLY))
    {
        message = "the DLPack tensor is read-only and cannot receive values";
        return false;
    }

    const DataKind kind = kindFromDLDataType(t->dtype);
    if (kind == DataKind::Unknown)
    {
        message = "DLPack element type (code " + std::to_string(t->dtype.code) +
                  ", " + std::to_string(t->dtype.bits) + " bits, " +
                  std::to_string(t->dtype.lanes) +
                  " lanes) has no HydroCouple DataKind";
        return false;
    }

    MemorySpace space;
    if (!spaceFromDeviceType(t->device.device_type, space))
    {
        message = "DLPack device type " +
                  std::to_string(t->device.device_type) + " is not known";
        return false;
    }

    if (t->byte_offset != 0 && isOpaqueHandleDevice(t->device.device_type))
    {
        message = "a non-zero byte_offset on a device whose data pointer is an "
                  "opaque handle cannot be folded into a BufferDescriptor";
        return false;
    }

    if (t->ndim < 0)
    {
        message = "DLPack tensor has negative rank";
        return false;
    }

    const int64_t itemBytes = itemSize(kind);
    borrow.stridesBytes.clear();
    if (t->ndim > 0 && t->strides != nullptr)
    {
        borrow.stridesBytes.reserve(static_cast<size_t>(t->ndim));
        for (int32_t k = 0; k < t->ndim; ++k)
            borrow.stridesBytes.push_back(t->strides[k] * itemBytes);
    }

    descriptor.data = static_cast<char *>(t->data) + t->byte_offset;
    descriptor.kind = kind;
    descriptor.rank = t->ndim;
    descriptor.shape = t->ndim > 0 ? t->shape : nullptr;
    // Pre-1.2 producers may leave strides null to mean C-contiguous, which is
    // exactly what a null stridesBytes means.
    descriptor.stridesBytes =
        borrow.stridesBytes.empty() ? nullptr : borrow.stridesBytes.data();
    descriptor.space = space;
    descriptor.deviceId = space == MemorySpace::Host ? 0 : t->device.device_id;
    return true;
}

// ---------------------------------------------------------------------------
// Producing: a BufferDescriptor in, a capsule out
// ---------------------------------------------------------------------------

/// Tensors this header exported whose deleter has not yet run.
inline std::atomic<int64_t> &liveExportsSlot()
{
    static std::atomic<int64_t> count{0};
    return count;
}

inline int64_t liveExports() { return liveExportsSlot().load(); }

/// One exported tensor: the managed tensor (either flavour) plus the shape
/// and element-stride storage it points at. The memory itself is borrowed.
struct Export
{
    DLManagedTensorVersioned versioned{};
    DLManagedTensor legacy{};
    std::vector<int64_t> shape;
    std::vector<int64_t> strides;
};

inline void deleteVersioned(DLManagedTensorVersioned *self)
{
    delete static_cast<Export *>(self->manager_ctx);
    liveExportsSlot().fetch_sub(1);
}

inline void deleteLegacy(DLManagedTensor *self)
{
    delete static_cast<Export *>(self->manager_ctx);
    liveExportsSlot().fetch_sub(1);
}

/// Capsule destructor: frees the tensor only if nobody consumed it. It can
/// run while an exception is propagating (the capsule is often the last
/// reference dropped during unwinding), so it must leave the error
/// indicator exactly as it found it.
inline void capsuleDestructor(PyObject *capsule)
{
    PyObject *type = nullptr, *value = nullptr, *traceback = nullptr;
    PyErr_Fetch(&type, &value, &traceback);
    if (PyCapsule_IsValid(capsule, "dltensor_versioned"))
    {
        auto *t = static_cast<DLManagedTensorVersioned *>(
            PyCapsule_GetPointer(capsule, "dltensor_versioned"));
        if (t && t->deleter)
            t->deleter(t);
    }
    else if (PyCapsule_IsValid(capsule, "dltensor"))
    {
        auto *t = static_cast<DLManagedTensor *>(
            PyCapsule_GetPointer(capsule, "dltensor"));
        if (t && t->deleter)
            t->deleter(t);
    }
    PyErr_Restore(type, value, traceback);
}

/// A new DLPack capsule over a descriptor's memory, without copying it.
/// `versioned` selects DLManagedTensorVersioned (DLPack >= 1.0 consumers)
/// over the legacy DLManagedTensor. `byteOffset` is written to the tensor
/// as-is (descriptor.data is then the base the offset is counted from); the
/// bindings' own views always pass 0, and tests use it to impersonate
/// producers that do not. Returns nullptr with `message` set on failure;
/// requires the GIL.
inline PyObject *makeCapsule(const BufferDescriptor &descriptor,
                             DLDevice device, bool readOnly, bool versioned,
                             std::string &message, uint64_t byteOffset = 0)
{
    DLDataType type;
    if (!dlDataTypeFromKind(descriptor.kind, type))
    {
        message = "DataKind " + std::to_string(static_cast<int>(descriptor.kind)) +
                  " cannot be expressed in DLPack";
        return nullptr;
    }
    if (descriptor.rank < 0)
    {
        message = "descriptor has negative rank";
        return nullptr;
    }
    if (descriptor.rank > 0 && descriptor.shape == nullptr)
    {
        message = "descriptor has a rank but no shape";
        return nullptr;
    }

    const int64_t itemBytes = type.bits / 8;
    auto *e = new Export();
    e->shape.assign(descriptor.shape, descriptor.shape + descriptor.rank);
    e->strides.resize(static_cast<size_t>(descriptor.rank));
    if (descriptor.stridesBytes != nullptr)
    {
        for (int32_t k = 0; k < descriptor.rank; ++k)
        {
            if (descriptor.stridesBytes[k] % itemBytes != 0)
            {
                delete e;
                message = "byte stride " +
                          std::to_string(descriptor.stridesBytes[k]) +
                          " is not a whole number of elements";
                return nullptr;
            }
            e->strides[static_cast<size_t>(k)] =
                descriptor.stridesBytes[k] / itemBytes;
        }
    }
    else
    {
        // DLPack >= 1.2 forbids null strides: spell out C-contiguous.
        int64_t step = 1;
        for (int32_t k = descriptor.rank - 1; k >= 0; --k)
        {
            e->strides[static_cast<size_t>(k)] = step;
            step *= e->shape[static_cast<size_t>(k)];
        }
    }

    DLTensor tensor;
    tensor.data = descriptor.data;
    tensor.device = device;
    tensor.ndim = descriptor.rank;
    tensor.dtype = type;
    tensor.shape = descriptor.rank > 0 ? e->shape.data() : nullptr;
    tensor.strides = descriptor.rank > 0 ? e->strides.data() : nullptr;
    tensor.byte_offset = byteOffset;

    PyObject *capsule = nullptr;
    if (versioned)
    {
        e->versioned.version.major = DLPACK_MAJOR_VERSION;
        e->versioned.version.minor = DLPACK_MINOR_VERSION;
        e->versioned.manager_ctx = e;
        e->versioned.deleter = &deleteVersioned;
        e->versioned.flags = readOnly ? DLPACK_FLAG_BITMASK_READ_ONLY : 0;
        e->versioned.dl_tensor = tensor;
        liveExportsSlot().fetch_add(1);
        capsule = PyCapsule_New(&e->versioned, "dltensor_versioned",
                                &capsuleDestructor);
    }
    else
    {
        e->legacy.manager_ctx = e;
        e->legacy.deleter = &deleteLegacy;
        e->legacy.dl_tensor = tensor;
        liveExportsSlot().fetch_add(1);
        capsule = PyCapsule_New(&e->legacy, "dltensor", &capsuleDestructor);
    }
    if (capsule == nullptr)
    {
        delete e;
        liveExportsSlot().fetch_sub(1);
        message = "PyCapsule_New failed";
    }
    return capsule;
}

} // namespace DLPack
} // namespace Python
} // namespace HydroCouple

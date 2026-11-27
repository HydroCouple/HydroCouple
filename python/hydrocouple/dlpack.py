"""
DLPack interop for the HydroCouple data plane.

This module belongs to the Python bindings, not to the interface: the C++
interface is header-only, depends on the standard library alone, and never
mentions DLPack. What makes the two meet is that a ``BufferDescriptor`` is
already a ``DLTensor`` in all but name -- data pointer, element type, rank,
shape, strides, memory space, device id -- so a tensor from any framework
that speaks DLPack (PyTorch, JAX, CuPy, NumPy, TensorFlow, ...) can cross
the boundary *without being copied*, on the host or on a device.

Two directions:

**Python drives C++.** ``get_values_into`` / ``set_values_from`` on every
C++ data-item wrapper accept any object implementing ``__dlpack__``:

.. code-block:: python

    t = torch.empty((24, 100), dtype=torch.float64, device="cuda")
    ok, msg = output.get_values_into(t, (0, 0), (24, 100))
    # the C++ item wrote straight into t's device memory

The C++ item receives a ``BufferDescriptor`` whose ``space`` is
``MemorySpace.Device`` and whose ``data`` is the tensor's own pointer. An
item that cannot service device memory refuses with a message, exactly as
the interface specifies; nothing is staged through the host behind your
back. The call is host-synchronous: when it returns, the values are in
place.

**C++ drives Python.** When a C++ workflow calls ``get_values_into`` on a
*Python* data item with a device descriptor, the Python method receives a
:class:`BufferView` -- ``torch.from_dlpack(view)`` gives a tensor that
aliases the caller's memory. Host-accessible descriptors still arrive as
NumPy arrays (which themselves implement ``__dlpack__``).

Devices. :class:`~hydrocouple.core.MemorySpace` is vendor neutral; DLPack
is not. Importing forgets the vendor (every accelerator is
``MemorySpace.Device``). Exporting needs it back, and takes it from
:func:`accelerator`, which defaults to CUDA; call :func:`set_accelerator`
once at start-up on ROCm, oneAPI or Metal.

The mappings themselves live in one place, the bindings' C++
``dlpack_bridge.h``; the functions here call it rather than re-derive it.
"""

from __future__ import annotations

from enum import IntEnum
from typing import Optional, Sequence

from hydrocouple.core import DataKind, MemorySpace

__all__ = [
    "DLDeviceType",
    "BufferView",
    "accelerator",
    "set_accelerator",
    "space_of",
    "device_of",
]


class DLDeviceType(IntEnum):
    """DLPack ``DLDeviceType`` codes (DLPack 1.3)."""

    CPU = 1
    CUDA = 2
    CUDAHost = 3
    OpenCL = 4
    Vulkan = 7
    Metal = 8
    VPI = 9
    ROCM = 10
    ROCMHost = 11
    ExtDev = 12
    CUDAManaged = 13
    OneAPI = 14
    WebGPU = 15
    Hexagon = 16
    MAIA = 17
    Trn = 18


def _core():
    from _hydrocouple import _core
    return _core


def accelerator() -> DLDeviceType:
    """The DLPack device type that ``MemorySpace.Device`` exports as."""
    return DLDeviceType(_core()._accelerator())


def set_accelerator(device_type: "DLDeviceType | int") -> None:
    """Choose what ``MemorySpace.Device`` means when exporting.

    Process wide. Raises :class:`ValueError` for a device type that is not
    an accelerator (CPU, pinned host, managed memory).
    """
    if not _core()._set_accelerator(int(device_type)):
        raise ValueError(
            f"{device_type!r} is not an accelerator device type")


def space_of(device_type: "DLDeviceType | int") -> Optional[MemorySpace]:
    """The :class:`MemorySpace` a DLPack device type denotes, or ``None``."""
    value = _core()._space_from_device_type(int(device_type))
    return None if value is None else MemorySpace(value)


def device_of(space: MemorySpace,
              device_id: int = 0) -> tuple[DLDeviceType, int]:
    """The ``(DLDeviceType, device_id)`` a memory space exports as."""
    device_type, device_id = _core()._device_from_space(int(space),
                                                       int(device_id))
    return DLDeviceType(device_type), device_id


class BufferView:
    """A borrowed, typed, strided buffer that speaks DLPack.

    What a Python data item receives in place of an ndarray when C++ hands
    it memory the host cannot touch. It owns nothing: ``__dlpack__`` wraps
    the same address every time, and the view is *invalidated* when the
    call it was lent for returns, after which ``__dlpack__`` raises
    ``BufferError``. A tensor made from it before then still aliases the
    memory and must not be kept past the call either -- the same rule as
    for the ndarray views the bridge hands out for host memory.

    It can also be built directly, e.g. to lend memory that C++ owns to a
    framework; the caller then guarantees the memory's lifetime.
    """

    __slots__ = ("_address", "_kind", "_shape", "_strides_bytes", "_space",
                 "_device_id", "_writable", "_valid")

    def __init__(self, address: int, data_kind: DataKind,
                 shape: Sequence[int],
                 strides_bytes: Optional[Sequence[int]] = None,
                 space: MemorySpace = MemorySpace.Device,
                 device_id: int = 0, writable: bool = True):
        self._address = int(address)
        self._kind = DataKind(data_kind)
        self._shape = tuple(int(s) for s in shape)
        self._strides_bytes = (None if strides_bytes is None
                               else tuple(int(s) for s in strides_bytes))
        if (self._strides_bytes is not None
                and len(self._strides_bytes) != len(self._shape)):
            raise ValueError("strides_bytes and shape differ in length")
        self._space = MemorySpace(space)
        self._device_id = int(device_id)
        self._writable = bool(writable)
        self._valid = True

    # -- description -----------------------------------------------------

    @property
    def data_ptr(self) -> int:
        """Address (or device pointer) of element ``(0, ..., 0)``."""
        return self._address

    @property
    def data_kind(self) -> DataKind:
        return self._kind

    @property
    def shape(self) -> tuple[int, ...]:
        return self._shape

    @property
    def strides_bytes(self) -> Optional[tuple[int, ...]]:
        """Byte strides, or ``None`` for C-contiguous."""
        return self._strides_bytes

    @property
    def space(self) -> MemorySpace:
        return self._space

    @property
    def device_id(self) -> int:
        return self._device_id

    @property
    def writable(self) -> bool:
        return self._writable

    @property
    def valid(self) -> bool:
        """False once the call this view was lent for has returned."""
        return self._valid

    def _invalidate(self) -> None:
        self._valid = False

    # -- DLPack producer -------------------------------------------------

    def __dlpack_device__(self) -> tuple[DLDeviceType, int]:
        return device_of(self._space, self._device_id)

    def __dlpack__(self, *, stream=None, max_version=None, dl_device=None,
                   copy=None):
        """A DLPack capsule over the viewed memory (never a copy).

        ``stream`` is accepted and not acted on: the bridge's contract is
        host-synchronous, so the memory is ready on every stream by the
        time a Python item sees it.
        """
        if not self._valid:
            raise BufferError(
                "this BufferView has outlived the call it was lent for")
        if copy:
            raise BufferError("a BufferView lends memory; it cannot copy")
        if dl_device is not None and (
                (int(dl_device[0]), int(dl_device[1]))
                != tuple(int(v) for v in self.__dlpack_device__())):
            raise BufferError(
                "a BufferView cannot move memory to another device")
        versioned = max_version is not None and int(max_version[0]) >= 1
        if not versioned and not self._writable:
            # A legacy capsule cannot say read-only; refuse rather than lie.
            raise BufferError(
                "a read-only BufferView needs a DLPack >= 1.0 consumer")
        device_type, device_id = self.__dlpack_device__()
        return _core()._make_capsule(
            self._address, int(self._kind), self._shape, self._strides_bytes,
            int(device_type), int(device_id), not self._writable, versioned)

    def __repr__(self) -> str:
        state = "" if self._valid else ", invalidated"
        return (f"BufferView(0x{self._address:x}, {self._kind.name}, "
                f"shape={self._shape}, {self._space.name}:{self._device_id}"
                f"{'' if self._writable else ', read-only'}{state})")
